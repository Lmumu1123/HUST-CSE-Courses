// pcap-sample.c

#include <string.h>
#include <stdlib.h>
#include <pcap.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/md5.h> // 为了MD5_DIGEST_LENGTH

#include "packet_header.h"
#include "TCPFlowReconstruction.h"

// 调试相关的宏定义
#define WITH_DBG
#include "se_dbg.h"
#define _DBG_FTP_CTRL

// FTP命令宏定义
#define FTP_CMD_PORT	"PORT "
#define FTP_CMD_USER	"USER "
#define FTP_CMD_PASS	"PASS "
#define FTP_CMD_PASV	"PASV"
#define FTP_CMD_LIST	"LIST"
#define FTP_CMD_RETR	"RETR "
#define FTP_CMD_STOR	"STOR "

// 最大并发会话数
#define MAX_SESSIONS 128

// 全局会话数组和计数器
ftp_session_t sessions[MAX_SESSIONS];
int session_count = 0;

// 函数: 初始化会话数组
void initialize_sessions() {
    memset(sessions, 0, sizeof(sessions));
    session_count = 0;
}

// 函数: 根据TCP四元组查找或创建新的FTP会话
ftp_session_t* find_or_create_session(uint32_t saddr, uint16_t sport, uint32_t daddr, uint16_t dport, int is_syn) {
    // 检查是否为已知的控制连接
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i].is_active) {
            if ((sessions[i].client_ip == saddr && sessions[i].client_port == sport && sessions[i].server_ip == daddr && sessions[i].server_port == dport) ||
                (sessions[i].client_ip == daddr && sessions[i].client_port == dport && sessions[i].server_ip == saddr && sessions[i].server_port == sport)) {
                return &sessions[i];
            }
        }
    }

    // 如果是FTP控制端口(21)的新连接(SYN)，则创建新会话
    if (is_syn && (dport == htons(21) || sport == htons(21))) {
        if (session_count < MAX_SESSIONS) {
            ftp_session_t* new_session = &sessions[session_count];
            memset(new_session, 0, sizeof(ftp_session_t));
            new_session->is_active = 1;
            if (dport == htons(21)) { // 客户端 -> 服务器
                new_session->client_ip = saddr; new_session->client_port = sport;
                new_session->server_ip = daddr; new_session->server_port = dport;
            } else { // 服务器 -> 客户端
                new_session->client_ip = daddr; new_session->client_port = dport;
                new_session->server_ip = saddr; new_session->server_port = sport;
            }
            session_count++;
            return new_session;
        }
    }
    
    // 检查是否为已知的数据连接
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (sessions[i].data_conn_active) {
             if ((sessions[i].data_conn_client_ip == saddr && sessions[i].data_conn_client_port == sport && sessions[i].data_conn_server_ip == daddr && sessions[i].data_conn_server_port == dport) ||
                (sessions[i].data_conn_client_ip == daddr && sessions[i].data_conn_client_port == dport && sessions[i].data_conn_server_ip == saddr && sessions[i].data_conn_server_port == sport)) {
                return &sessions[i];
            }
        }
    }

    // 如果是新的SYN包，检查是否匹配某个会话等待建立的数据连接
    if (is_syn) {
        for (int i = 0; i < MAX_SESSIONS; i++) {
            if (sessions[i].is_active && sessions[i].data_listen_port != 0) {
                 // 被动模式: 客户端连接服务器指定的IP和端口
                if (strcmp(sessions[i].data_mode, "PASV") == 0 && daddr == sessions[i].data_listen_ip && dport == sessions[i].data_listen_port) {
                    sessions[i].data_conn_active = 1;
                    sessions[i].data_conn_client_ip = saddr; sessions[i].data_conn_client_port = sport;
                    sessions[i].data_conn_server_ip = daddr; sessions[i].data_conn_server_port = dport;
                    return &sessions[i];
                }
                // 主动模式: 服务器应该连接客户端
                if (strcmp(sessions[i].data_mode, "PORT") == 0 && daddr == sessions[i].data_listen_ip && dport == sessions[i].data_listen_port) {
                     sessions[i].data_conn_active = 1;
                     sessions[i].data_conn_client_ip = daddr; sessions[i].data_conn_client_port = dport; // Client is the one being connected to
                     sessions[i].data_conn_server_ip = saddr; sessions[i].data_conn_server_port = sport; // Server is the one initiating
                    return &sessions[i];
                }
            }
        }
    }

    return NULL;
}


// 函数: 从PORT或PASV响应中提取IP和端口
int get_ftp_data_addr(const char *addrstr, ftp_session_t* session);

// 函数: 处理FTP控制连接的消息
void ftp_ctrl_proc(int dir, ftp_session_t *session, const u_char * ftp_msg, __u32 msg_len)
{
	const char *addrstr = NULL;

	if (msg_len == 0 || session == NULL)
		return;
    
    char msg_buf[msg_len + 1];
    strncpy(msg_buf, (const char *)ftp_msg, msg_len);
    msg_buf[msg_len] = '\0';

    char* crlf = strstr(msg_buf, "\r\n");
    if (crlf) *crlf = '\0';


#ifdef _DBG_FTP_CTRL
	DBG("FTP-CTRL: ");
	if (dir == 0) {
		DBG("C->S: %s\n", msg_buf);
	} else {
		DBG("S->C: %s\n", msg_buf);
	}
#endif

	if (strncmp(msg_buf, FTP_CMD_USER, strlen(FTP_CMD_USER)) == 0) {
        strncpy(session->username, msg_buf + strlen(FTP_CMD_USER), sizeof(session->username) - 1);
    } else if (strncmp(msg_buf, FTP_CMD_PASS, strlen(FTP_CMD_PASS)) == 0) {
        strncpy(session->password, msg_buf + strlen(FTP_CMD_PASS), sizeof(session->password) - 1);
    } else if (strncmp(msg_buf, FTP_CMD_PORT, strlen(FTP_CMD_PORT)) == 0) {
		// 主动模式: "PORT a1,a2,a3,a4,a5,a6"
		addrstr = msg_buf + strlen(FTP_CMD_PORT);
		if (get_ftp_data_addr(addrstr, session) == 0) {
            // --- 修改点 ---
			strncpy(session->data_mode, "PORT", sizeof(session->data_mode) - 1);
			DBG("***** FTP Data Mode set to PORT, Client listening on: %s:%u\n", session->data_listen_ip_str, ntohs(session->data_listen_port));
		}
	} else if (strncmp(msg_buf, "227", 3) == 0) {
		// 被动模式: "227 Entering Passive Mode (a1,a2,a3,a4,a5,a6)"
		addrstr = strchr(msg_buf, '(');
		if (addrstr != NULL) {
			addrstr++;
			if (get_ftp_data_addr(addrstr, session) == 0) {
                // --- 修改点 ---
				strncpy(session->data_mode, "PASV", sizeof(session->data_mode) - 1);
                DBG("***** FTP Data Mode set to PASV, Server listening on: %s:%u\n", session->data_listen_ip_str, ntohs(session->data_listen_port));
			}
		}
	} else if (strncmp(msg_buf, FTP_CMD_RETR, strlen(FTP_CMD_RETR)) == 0) {
        strncpy(session->filename, msg_buf + strlen(FTP_CMD_RETR), sizeof(session->filename) - 1);
        DBG("***** Get file command detected: %s\n", session->filename);
    } else if (strncmp(msg_buf, FTP_CMD_STOR, strlen(FTP_CMD_STOR)) == 0) {
        strncpy(session->filename, msg_buf + strlen(FTP_CMD_STOR), sizeof(session->filename) - 1);
        DBG("***** Put file command detected: %s\n", session->filename);
    } else if (strncmp(msg_buf, FTP_CMD_LIST, strlen(FTP_CMD_LIST)) == 0) {
        strncpy(session->filename, "(Directory Listing)", sizeof(session->filename) - 1);
        DBG("***** List command detected.\n");
    }
	return;
}

// 函数: 处理TCP数据包
void tcp_proc(const u_char * tcp_pkt, __u32 pkt_len, __u32 srcip, __u32 dstip)
{
	TCPHdr_t *tcph = (TCPHdr_t *) tcp_pkt;
    uint16_t sport = tcph->source;
    uint16_t dport = tcph->dest;
    
	const u_char *payload = tcp_pkt + tcph->doff * 4;
	__u32 payload_len = pkt_len - tcph->doff * 4;

    // 查找或创建会话
    ftp_session_t* session = find_or_create_session(srcip, sport, dstip, dport, tcph->syn);
    if (!session) return;


	// 判断是控制连接还是数据连接
    if (session->server_port == dport || session->server_port == sport) { // 控制连接
        if (payload_len > 0) {
            int dir = (session->client_ip == srcip) ? 0 : 1; // 0 for C->S, 1 for S->C
            ftp_ctrl_proc(dir, session, payload, payload_len);
        }
    } else if (session->data_conn_active) { // 数据连接
	    __u32 seq = ntohl(tcph->seq);
		process_ftp_data(session, payload, payload_len, seq, tcph->syn, tcph->fin);
	}

	return;
};

// 函数: 处理IP数据包
void ip_proc(const u_char * ip_pkt, __u32 pkt_len)
{
	IPHdr_t *iph = (IPHdr_t *) ip_pkt;

	if (iph->protocol == IPPROTO_TCP) {
		tcp_proc(ip_pkt + iph->ihl * 4, ntohs(iph->tot_len) - iph->ihl * 4, iph->saddr, iph->daddr);
		return;
	}

	return;
}

// 函数: pcap_loop的回调函数，处理每个抓到的包
void pkt_proc(u_char * arg, const struct pcap_pkthdr *pkthdr, const u_char * packet)
{
	int *cnt = (int *) arg;
	EthHdr_t *eth = (EthHdr_t *) packet;

	(*cnt)++;

	if (ntohs(eth->h_type) == 0x0800) { // 如果是以太网帧类型为IP (0x0800)
		ip_proc(packet + sizeof(EthHdr_t), pkthdr->len - sizeof(EthHdr_t));
		return;
	}

	return;
}

// 函数: 打印程序用法
void usage(const char *appname)
{
	printf("Usage:\n");
	printf("\t%s <pcap filename>\n", appname);
    printf("Compile with: gcc pcap-sample.c TCPFlowReconstruction.c -o ftp_analyzer -lpcap -lcrypto\n");
	return;
}

// 函数: 格式化打印最终收集到的会话信息
void print_final_session_info() {
    printf("\n\n");
    printf("############################################################\n");
    printf("#              FTP Session Analysis Report                 #\n");
    printf("############################################################\n");
    
    if (session_count == 0) {
        printf("\nNo FTP sessions were found in the capture file.\n\n");
        return;
    }

    for (int i = 0; i < session_count; i++) {
        ftp_session_t* s = &sessions[i];
        char client_ip_str[INET_ADDRSTRLEN];
        char server_ip_str[INET_ADDRSTRLEN];
        
        inet_ntop(AF_INET, &s->client_ip, client_ip_str, sizeof(client_ip_str));
        inet_ntop(AF_INET, &s->server_ip, server_ip_str, sizeof(server_ip_str));

        printf("\n- - - - - - - - - - [ Session %-2d ] - - - - - - - - - -\n", i + 1);
        printf("  %-25s : %s\n", "Client IP", client_ip_str);
        printf("  %-25s : %s\n", "Server IP", server_ip_str);
        printf("  %-25s : %s\n", "Username", strlen(s->username) > 0 ? s->username : "N/A");
        printf("  %-25s : %s\n", "Password", strlen(s->password) > 0 ? s->password : "N/A");
        
        if (strlen(s->data_mode) > 0) {
            printf("  %-25s : %s\n", "Data Connection Mode", s->data_mode);
            printf("  %-25s : %s\n", "Data Connection IP", s->data_listen_ip_str);
            printf("  %-25s : %u\n", "Data Connection Port", ntohs(s->data_listen_port));
        } else {
             printf("  %-25s : N/A\n", "Data Connection Info");
        }
       
        if (strlen(s->last_filename) > 0) {
            printf("  %-25s : %s\n", "Last Transferred Object", s->last_filename);
            printf("  %-25s : %s\n", "Object MD5", strlen(s->last_file_md5_str) > 0 ? s->last_file_md5_str : "N/A (or not a file transfer)");
        } else {
            printf("  %-25s : None detected\n", "Data Transfers");
        }
        printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
    }
     printf("\n############################################################\n\n");
}


// 函数: 主函数
int main(int argc, char **argv)
{
	char *pfile;
	pcap_t *pd = NULL;
	char ebuf[PCAP_ERRBUF_SIZE];
	int count = 0;

	if (argc != 2) {
		usage(argv[0]);
		return -1;
	}

	pfile = argv[1];
	printf("Analyzing pcap file: %s\n", pfile);

    initialize_sessions();

	// 打开一个已保存的pcap文件
	pd = pcap_open_offline(pfile, ebuf);
	if (pd == NULL) {
		printf("Error: Failed to open pcap file (%s)\n", ebuf);
		return -1;
	}

	// 无限循环，处理每个数据包
	pcap_loop(pd, -1, pkt_proc, (u_char *) & count);

	printf("\n============================================================\n");
	printf("Total %d packets were analyzed.\n", count);
    
    // 最终打印所有会话信息
    print_final_session_info();

	pcap_close(pd);
	return 0;
}
