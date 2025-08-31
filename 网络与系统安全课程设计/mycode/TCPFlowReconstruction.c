// TCPFlowReconstruction.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <openssl/evp.h> // 使用EVP API进行MD5计算
#include <openssl/md5.h> // 包含此头文件以定义 MD5_DIGEST_LENGTH
#include "TCPFlowReconstruction.h"

// 函数: 从PORT命令参数中提取IP和端口
int get_ftp_data_addr(const char *addrstr, ftp_session_t* session) {
    unsigned int a1, a2, a3, a4, p1, p2;
    struct in_addr in;

    if (sscanf(addrstr, "%u,%u,%u,%u,%u,%u", &a1, &a2, &a3, &a4, &p1, &p2) != 6) return -1;
    
    sprintf(session->data_listen_ip_str, "%u.%u.%u.%u", a1, a2, a3, a4);
    if (inet_aton(session->data_listen_ip_str, &in) == 0) return -1;

    session->data_listen_ip = in.s_addr;
    session->data_listen_port = htons(p1 * 256 + p2); // 存储为网络字节序
    return 0;
}

// 函数: 将二进制MD5哈希值转换为十六进制字符串
static void md5_to_string(char *out, const unsigned char *md5) {
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(out + i * 2, "%02x", md5[i]);
    }
    out[MD5_DIGEST_LENGTH * 2] = '\0';
}

// 函数: 向会话的流链表中插入一个数据段 (按序号排序，忽略重复数据)
static void insert_segment(ftp_session_t* session, __u32 seq, const u_char *data, __u32 len) {
    segment_t *seg, *prev = NULL, *cur = session->stream_head;

    if (len == 0) return; // 忽略空载荷的数据包

    while(cur) {
        if(cur->seq == seq) return; // 忽略重复的数据包
        if(cur->seq > seq) break;
        prev = cur;
        cur = cur->next;
    }

    seg = (segment_t *)malloc(sizeof(segment_t));
    if (!seg) { perror("malloc segment"); return; }
    seg->seq = seq;
    seg->len = len;
    seg->data = (u_char *)malloc(len);
    if (!seg->data) { perror("malloc data"); free(seg); return; }
    memcpy(seg->data, data, len);
    seg->next = cur;

    if(prev) prev->next = seg;
    else session->stream_head = seg;
}

// --- 新增函数 ---
// 函数: 将指定会话中重组好的数据流写入文件
static void write_reconstructed_file(ftp_session_t* session) {
    // 检查是否存在有效的文件名，并确保不是目录列表操作
    if (session->filename[0] == '\0' || strcmp(session->filename, "(Directory Listing)") == 0) {
        return;
    }

    // 使用 "wb" (二进制写入模式) 打开文件
    FILE *fp = fopen(session->filename, "wb");
    if (!fp) {
        // 如果文件打开失败，打印错误信息
        perror("Error opening file for writing");
        return;
    }

    printf("Info: Writing reconstructed data to file '%s'...\n", session->filename);

    // 遍历重组后的数据链表，并将每个数据段写入文件
    segment_t *cur = session->stream_head;
    while (cur) {
        fwrite(cur->data, 1, cur->len, fp);
        cur = cur->next;
    }

    fclose(fp);
    printf("Info: Successfully wrote file '%s'.\n", session->filename);
}


// 函数: 释放会话的流链表所占用的内存
static void free_stream(ftp_session_t* session) {
    segment_t *cur = session->stream_head, *tmp;
    while(cur) {
        tmp = cur;
        cur = cur->next;
        free(tmp->data);
        free(tmp);
    }
    session->stream_head = NULL;
    session->stream_initialized = 0;
}

// 函数: 处理FTP数据流的核心函数
void process_ftp_data(ftp_session_t *session, const u_char *payload, __u32 payload_len, uint32_t seq, int syn, int fin) {
    if (!session) return;
    unsigned int md_len;

    // 如果SYN置位，初始化流状态
    if(syn && !session->stream_initialized) {
        free_stream(session); // 确保没有旧数据
        session->stream_initialized = 1;
    }

    if(!session->stream_initialized) return;

    // 插入当前数据段
    insert_segment(session, seq, payload, payload_len);
    
    // --- MD5 计算 ---
    if (payload_len > 0) {
        if (!session->md5_initialized) { // 如果是此数据连接的第一个数据包
            session->md_ctx = EVP_MD_CTX_new();
            EVP_DigestInit_ex(session->md_ctx, EVP_md5(), NULL);
            session->md5_initialized = 1;
        }
        EVP_DigestUpdate(session->md_ctx, payload, payload_len); // 更新哈希值
    }

    // 如果FIN置位，认为流结束
    if(fin) {
        // --- 新增调用 ---
        // 在完成所有操作、释放内存之前，先将重组好的数据写入文件
        write_reconstructed_file(session);

        // 完成MD5计算并存储结果
        if (session->md5_initialized) {
            EVP_DigestFinal_ex(session->md_ctx, session->file_md5, &md_len);
            EVP_MD_CTX_free(session->md_ctx); // 释放上下文
            
            // 将本次传输的结果保存到会话的永久存储中
            md5_to_string(session->last_file_md5_str, session->file_md5);
            strncpy(session->last_filename, session->filename, sizeof(session->last_filename) - 1);
        }
        
        // --- 关键修复：重置所有与本次数据传输相关的状态 ---
        free_stream(session);                // 释放链表内存
        session->data_conn_active = 0;       // 标记数据连接为非活动
        session->md5_initialized = 0;        // 重置MD5初始化标志
        session->md_ctx = NULL;              // 将指针置空，防止意外使用
        memset(session->filename, 0, sizeof(session->filename)); // 清空当前文件名
    }
}
