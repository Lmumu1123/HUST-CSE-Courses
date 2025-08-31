// TCPFlowReconstruction.h

#ifndef TCP_FLOW_RECONSTRUCTION_H
#define TCP_FLOW_RECONSTRUCTION_H

#include <netinet/in.h>
#include <pcap.h>
#include <asm/types.h>
#include <openssl/evp.h> // 包含EVP头文件

// 前向声明数据段结构体
struct segment;

// FTP会话的完整上下文结构体
typedef struct {
    // 会话标识
    uint32_t client_ip;
    uint16_t client_port;
    uint32_t server_ip;
    uint16_t server_port;
    int is_active;

    // 收集的信息
    char username[256];
    char password[256];
    char filename[256];
    char data_mode[16];
    uint32_t data_listen_ip;
    uint16_t data_listen_port;
    char data_listen_ip_str[INET_ADDRSTRLEN]; // IP地址的字符串形式

    // 永久存储最后一次传输的文件信息
    char last_filename[256];
    unsigned char file_md5[16]; // MD5哈希值是16字节
    char last_file_md5_str[33]; // MD5字符串形式 "xx...xx\0"

    // 流重组状态
    struct segment *stream_head;
    int stream_initialized;
    
    // MD5计算状态
    EVP_MD_CTX *md_ctx;
    int md5_initialized;

    // 数据连接状态
    uint32_t data_conn_client_ip;
    uint16_t data_conn_client_port;
    uint32_t data_conn_server_ip;
    uint16_t data_conn_server_port;
    int data_conn_active;

} ftp_session_t;

// TCP数据段结构体
typedef struct segment {
    __u32 seq;
    __u32 len;
    u_char *data;
    struct segment *next;
} segment_t;


// 函数: 处理FTP数据连接的TCP载荷
// 参数: session - 当前FTP会话的上下文指针
//       payload – TCP载荷数据; payload_len – 数据长度
//       seq – TCP序列号; syn/fin – 标识SYN/FIN标志（非0表示置位）
void process_ftp_data(ftp_session_t *session, const u_char *payload, __u32 payload_len, uint32_t seq, int syn, int fin);

// 函数: 从字符串中解析FTP数据连接的地址
int get_ftp_data_addr(const char *addrstr, ftp_session_t* session);


#endif
