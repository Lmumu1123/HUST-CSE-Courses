#include <string.h>
#include <stdlib.h>
#include <pcap.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "packet_header.h"

#define WITH_DBG
#include "se_dbg.h"

//#define _DBG_PKT
//#define _DBG_ETH
//#define _DBG_IP
//#define _DBG_TCP
#define _DBG_FTP_CTRL

#define FTP_CMD_PORT	"PORT "
#define FTP_CMD_PASV	"PASV"
#define FTP_CMD_LIST	"LIST"
#define FTP_CMD_RETR	"RETR "
#define FTP_CMD_STOR	"STOR "

#define FTP_DATA_CMD_LIST	1
#define FTP_DATA_CMD_RETR	2
#define FTP_DATA_CMD_STOR	3
static __u32 ftp_data_cmd = 0;
static char ftp_filename[256];

#define FTP_DATA_MODE_PORT	1
#define FTP_DATA_MODE_PASV	2
static __u32 ftp_data_mode = 0;

static __u32 ftp_data_listen_ip = 0;
static __u16 ftp_data_listen_port = 0;

//transform "a1,a2,a3,a4,a5,a6" to IP and port
int get_ftp_data_addr(const char *addrstr)
{
	__u32 a1, a2, a3, a4, a5, a6;
	char ipstr[20];
	struct in_addr in;

	if (addrstr == NULL)
		goto errout;

	sscanf(addrstr, "%u,%u,%u,%u,%u,%u", &a1, &a2, &a3, &a4, &a5, &a6);
	sprintf(ipstr, "%u.%u.%u.%u", a1, a2, a3, a4);
	if (inet_aton(ipstr, &in) < 0)
		goto errout;

	ftp_data_listen_ip = in.s_addr;
	ftp_data_listen_port = a5 * 256 + a6;
	return 0;

  errout:
	ftp_data_listen_ip = 0;
	ftp_data_listen_port = 0;
	return -1;
}

void ftp_ctrl_proc(int dir, const u_char * ftp_msg, __u32 msg_len)
{
	const char *addrstr = NULL;

	if (msg_len == 0)
		return;

#ifdef _DBG_FTP_CTRL
	DBG("FTP-CTRL: ");
	if (dir == 0) {
		DBG("C->S: %.*s", msg_len, (char *) ftp_msg);
	} else {
		DBG("S->C: %.*s", msg_len, (char *) ftp_msg);
	}
#endif

	if (strncmp(ftp_msg, FTP_CMD_PORT, strlen(FTP_CMD_PORT)) == 0) {
		//"PORT a1,a2,a3,a4,a5,a6
		addrstr = ftp_msg + strlen(FTP_CMD_PORT);
		if (get_ftp_data_addr(addrstr) == 0) {
			ftp_data_mode = FTP_DATA_MODE_PORT;
			DBG("***** FTP DATA Mode: %d, Server: %u.%u.%u.%u:%u\n", ftp_data_mode, NIPQUAD(ftp_data_listen_ip), ftp_data_listen_port);
		}
	} else if (strncmp(ftp_msg, "227", strlen("227")) == 0) {
		//"227 Entering Passive Mode (a1,a2,a3,a4,a5,a6)"
		addrstr = strchr(ftp_msg, '(');
		if (addrstr != NULL) {
			addrstr++;
			if (get_ftp_data_addr(addrstr) == 0) {
				ftp_data_mode = FTP_DATA_MODE_PASV;
				DBG("***** FTP DATA Mode: %d, Server: %u.%u.%u.%u:%u\n", ftp_data_mode, NIPQUAD(ftp_data_listen_ip), ftp_data_listen_port);
			}
		}
	}

	if (ftp_data_mode) {
		if (strncmp(ftp_msg, FTP_CMD_LIST, strlen(FTP_CMD_LIST)) == 0) {
			ftp_data_cmd = FTP_DATA_CMD_LIST;
			bzero(ftp_filename, sizeof(ftp_filename));
		} else if (strncmp(ftp_msg, FTP_CMD_RETR, strlen(FTP_CMD_RETR)) == 0) {
			ftp_data_cmd = FTP_DATA_CMD_RETR;
			bzero(ftp_filename, sizeof(ftp_filename));
			strncpy(ftp_filename, ftp_msg + strlen(FTP_CMD_RETR), msg_len - strlen(FTP_CMD_RETR) - 2);	//exclude tail "\r\n"
			DBG("***** Get file %s\n", ftp_filename);
		} else if (strncmp(ftp_msg, FTP_CMD_STOR, strlen(FTP_CMD_STOR)) == 0) {
			ftp_data_cmd = FTP_DATA_CMD_STOR;
			bzero(ftp_filename, sizeof(ftp_filename));
			strncpy(ftp_filename, ftp_msg + strlen(FTP_CMD_STOR), msg_len - strlen(FTP_CMD_STOR) - 2);	//exclude tail "\r\n"
			DBG("***** Put file %s\n", ftp_filename);
		}
	}
	return;
}

void tcp_proc(const u_char * tcp_pkt, __u32 pkt_len, __u32 srcip, __u32 dstip)
{
	TCPHdr_t *tcph = (TCPHdr_t *) tcp_pkt;

#ifdef _DBG_TCP
	DBG("**** TCP Header ****\n");
	DBG("Source Port: %d\n", ntohs(tcph->source));
	DBG("Dest   Port: %d\n", ntohs(tcph->dest));
	DBG("Data Offset: %d (%d bytes)\n", tcph->doff, tcph->doff * 4);
	DBG("SequenceNum: %u\n", ntohl(tcph->seq));
	DBG("Ack Number : %u\n", ntohl(tcph->ack_seq));
	DBG("TCP Payload: %u bytes\n", pkt_len - tcph->doff * 4);
	DBG("Flags      :");
	if (tcph->syn)
		DBG(" SYN");
	if (tcph->fin)
		DBG(" FIN");
	if (tcph->rst)
		DBG(" RST");
	if (tcph->ack)
		DBG(" ACK");
	DBG("\n\n");
#endif							//_DBG_TCP

	if (ntohs(tcph->dest) == 21) {
		ftp_ctrl_proc(0, tcp_pkt + tcph->doff * 4, pkt_len - tcph->doff * 4);
		return;
	} else if (ntohs(tcph->source) == 21) {
		ftp_ctrl_proc(1, tcp_pkt + tcph->doff * 4, pkt_len - tcph->doff * 4);
		return;
	}

	/* FTP data connection process */
	/* Add your code here */

	return;
};

void ip_proc(const u_char * ip_pkt, __u32 pkt_len)
{
	IPHdr_t *iph = (IPHdr_t *) ip_pkt;

#ifdef _DBG_IP
	DBG("*** IP Header ***\n");
	DBG("Version  : %d\n", iph->version);
	DBG("Headerlen: %d (%d bytes)\n", iph->ihl, iph->ihl * 4);
	DBG("Total len: %d\n", ntohs(iph->tot_len));
	DBG("Source IP: %d.%d.%d.%d\n", NIPQUAD(iph->saddr));
	DBG("Dest   IP: %d.%d.%d.%d\n", NIPQUAD(iph->daddr));
	DBG("Protocol : %d", iph->protocol);
	switch (iph->protocol) {
	case IPPROTO_ICMP:
		DBG("(ICMP)\n\n");
		break;
	case IPPROTO_TCP:
		DBG("(TCP)\n\n");
		break;
	case IPPROTO_UDP:
		DBG("(UDP)\n\n");
		break;
	default:
		DBG("(Other)\n\n");
		break;
	}
#endif							//_DBG_IP

	if (iph->protocol == IPPROTO_TCP) {
		tcp_proc(ip_pkt + iph->ihl * 4, ntohs(iph->tot_len) - iph->ihl * 4, iph->saddr, iph->daddr);
		return;
	}

	return;
}

void pkt_proc(u_char * arg, const struct pcap_pkthdr *pkthdr, const u_char * packet)
{
	int i = 0;
	int *cnt = (int *) arg;
	EthHdr_t *eth = (EthHdr_t *) packet;

	(*cnt)++;

#ifdef _DBG_PKT
	DBG("------------------------------------------------------------\n");
	DBG("Packet #%d (%dB): \n", (*cnt), pkthdr->len);
	DBG_DUMP_BYTES(packet, pkthdr->len);
#endif							//_DBG_PKT

#ifdef _DBG_ETH
	DBG("** Ether Header **\n");
	DBG("Dest   MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", MACSIX(eth->h_dest));

	DBG("Source MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", MACSIX(eth->h_source));
	DBG("Frame Type: 0x%04X(%s)\n\n", ntohs(eth->h_type), ((ntohs(eth->h_type) == 0x0800) ? "IP" : "Other"));
#endif							//_DBG_ETH

	if (ntohs(eth->h_type) == 0x0800) {
		ip_proc(packet + sizeof(EthHdr_t), pkthdr->len - sizeof(EthHdr_t));
		return;
	}

	return;
}

void usage(const char *appname)
{
	printf("Usage:\n");
	printf("\t%s <pcap filename>\n", appname);
	return;
}

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
	printf("pcap file: %s\n", pfile);

	/*
	 * Open a saved pcap file
	 */
	pd = pcap_open_offline(pfile, ebuf);
	if (pd == NULL) {
		printf("Open pcap file failed (%s)\n", ebuf);
		return -1;
	}

	/*
	 * Loop forever & process each packet 
	 */
	pcap_loop(pd, -1, pkt_proc, (u_char *) & count);

	printf("============================================================\n");
	printf("Total %d packets are analyzed.\n\n", count);
	pcap_close(pd);
	return 0;
}
