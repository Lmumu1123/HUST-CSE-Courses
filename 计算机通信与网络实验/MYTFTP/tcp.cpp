//g++ try.cpp -o try -lwsock32 -std=c++11
#include <winsock2.h>
#include <iostream>
#include <ws2tcpip.h>
#include <cstdlib>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

//生成并初始化一个UDPsocket

int get_UDP_socket()
{
    WORD ver = MAKEWORD(2,2);   //MAKEWORD(1,1)和MAKEWORD(2,2)的区别在于，前者只能一次接收一次，不能马上发送，而后者能。
    WSADATA  lp_data;
    int err = WSAStartup(ver,&lp_data);     //生成socket,返回值为0表示正常
    if(err != 0) return -1;
    int udp_socket = socket(AF_INET,SOCK_DGRAM,0);      //AF_INET指代TCP/IP-IPV4协议族
    if(udp_socket == INVALID_SOCKET) return -2;
    return udp_socket;
}

//根据输入的ip地址和端口号，构造出一个存储着地址的sockaddr_in类型
sockaddr_in get_addr(const char *ip,int port)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = inet_addr(ip);
    return addr;
}

//构造RRQ包(读请求包)
char* request_download_pack(char *content,int &datalen,int type)
{
    int len = strlen(content);
    char *buf = new char[len + 2 + 2 + type];       //提前分配需要的内存空间
    buf[0] = 0x00;
    buf[1] = 0x01;          //RRQ在TFTP中用0x01表示
    memcpy(buf + 2,content,len);    //将被请求的文件名content放入RRQ包
    memcpy(buf + 2 + len,"\0",1);   //文件名结尾加上一个结束符
    if(type == 5)               //根据用户规定的传输格式来构造数据包，共有两种：oetct，netascii
        memcpy(buf + 2 + len + 1,"octet",5);
    else
        memcpy(buf + 2 + len + 1,"netascii",8);
    memcpy(buf + 2 + len + 1 + type,"\0",1);        //传输格式结尾加上结束符
    datalen = 2 + len + 1 + type + 1;               //datalen是一个引用变量，通过这个变量将将数据长度传输出去
    return buf;
}

//构造WRQ包(写请求包)
char* request_upload_pack(char *content,int &datalen,int type)
{
    int len = strlen(content);
    char *buf = new char[len + 2 + 2 + type];
    buf[0] = 0x00;
    buf[1] = 0x02;          //WRQ在TFTP中用0x02表示
    memcpy(buf + 2,content,len);
    memcpy(buf + 2 + len,"\0",1);
    if(type == 5)
        memcpy(buf + 2 + len + 1,"octet",5);
    else
        memcpy(buf + 2 + len + 1,"netascii",8);
    memcpy(buf + 2 + len + 1 +type,"\0",1);
    datalen = 2 + len + 1 + type + 1;
    return buf;
}

//构造ACK数据包
char* ack_pack(short &no)
{
    char *ack = new char[4];
    ack[0] = 0x00;
    ack[1] = 0x04;          //ACK数据包在TFTP中用0x04表示
    no = htons(no);         //将主机的字节序转换为网络字节序，因为不同操作系统的存储方式不同，
    memcpy(ack + 2,&no,2);  //为了方便交流，规定了统一的网络传输字节序，由对应的主机根据自身
    no = ntohs(no);         //系统的存储规则转换为主机字节序
    return ack;             //ntohs与htons相反，ntohs将网络字节序变为主机字节序，htons将主机字节序变为网络字节序
}                           //no是引用类型的变量，所以必须要在传输完后恢复到在主机上存储的格式

//构造DATA数据包
char* data_pack(short &no,FILE *f,int &datalen)
{
    char tmp[512];
    int sum = fread(tmp,1,512,f);           //读入512字节的文件内容
    if(!ferror(f))
    {
        char *buf = new char[4 + sum];      //预先分配内存
        buf[0] = 0x00;
        buf[1] = 0x03;          //DATA在TFTP中用0x03表示
        no = htons(no);
        memcpy(buf + 2,&no,2);              //将no放入数据包
        no = ntohs(no);
        memcpy(buf + 4,tmp,sum);            //将文件内容放入数据包
        datalen = sum + 4;
        return buf;
    }
    else return NULL;
}

//按照“年-月-日-时-分-秒”的形式输出当前时间，用于构造日志文件
void print_time(FILE *fp)
{
    time_t t;
    time(&t);           //获取当前时间
    char s_time[100];
    strcpy(s_time,ctime(&t));           //ctime将time_t类型转化为人类易读的时间格式
    *(strchr(s_time,'\n')) = '\0';      //转化结果最后包含一个回车符，将其换成结束符
    fprintf(fp,"[ %s ] ",s_time);
    return;
}

int main()
{
    FILE *fp = fopen("TFTP_client.log","a");        //打开日志文件
    char repeat_buf[2048];
    int buf_len;
    int number_to_kill;             
    int kill_time;
    clock_t start,end;              //记录传输的开始时间和结束时间，用于计算传输速率
    double run_time;
    SOCKET sock = get_UDP_socket();     //创建套接字    
    sockaddr_in addr;
    int recv_timeout = 1000;        //1000ms
    int send_timeout = 1000;
    setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&recv_timeout,sizeof(int));   //该函数用于对socket设置一个读取超时时间
    setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&send_timeout,sizeof(int));   //一个是发送超时时间，一个是接收超时时间
    while(1)
    {
        printf("+------------------------------+\n");
        printf("|   1:upload files             |\n");
        printf("|   2:download files           |\n");
        printf("|   0:close TFTP client        |\n");
        printf("+------------------------------+\n");
        int choice;
        scanf("%d",&choice);
        if(choice == 1)         //上传
        {
            addr = get_addr("127.0.0.1",69);    //第一个数据包(RRQ或WRQ)总是发送到69端口，
            printf("input file fullname:\n");
            char fullname[1024];
            scanf("%s",fullname);
            printf("choose a way to upload files:\n");
            printf("1:netascii\n");
            printf("2:octet\n");
            int type;
            scanf("%d",&type);
            if(type == 1) type = 8;     //8对应netascii
            else type = 5;              //5对应octet
            int datalen;
            char *send_data = request_upload_pack(fullname,datalen,type);
            buf_len = datalen;
            number_to_kill = 1;             //记录一个数据包recv_from超时的次数
            memcpy(repeat_buf,send_data,datalen);   //repeat_buf用于数据重传，有可能重传的数据会放入其中
            int res = sendto(sock,send_data,datalen,0,(sockaddr*)&addr,sizeof(addr));   //首次发送WRQ包,res代表发出的长度
            start = clock();        //开始计时
            print_time(fp);
            fprintf(fp,"send WRQ for file: %s\n",fullname);
            kill_time = 1;          //表示sendto超时的次数，与recv_from分开计算
            while(res != datalen)       //如果没有全部sendto成功，重新sendto，直到成功或达到次数上限
            {
                cout << "send WRQ failed: " << kill_time << "times" << endl;
                if(kill_time <= 10)
                {
                    res = sendto(sock,repeat_buf,buf_len,0,(sockaddr*)&addr,sizeof(addr));
                    kill_time ++;
                }
                else break;
            }
            if(kill_time > 10) continue;        //传输失败
            delete[]send_data;
            FILE *f = fopen(fullname,"rb");
            if(f == NULL)
            {
                cout << "file " << fullname << " open failed" << endl;
                continue;
            }
            short block = 0;
            datalen = 0;
            int rst = 0;        //记录重传次数
            int full_size = 0;  //记录文件总大小
            while(1)        //开始传输
            {
                char buf[1024];
                sockaddr_in server;     //从server反馈的数据包中获得其分配的端口号信息
                int len = sizeof(server);
                res = recvfrom(sock,buf,1024,0,(sockaddr*)&server,&len);        //监听服务器的数据包
                if(res == -1)       //如果没有收到数据包
                {
                    printf("%d ",number_to_kill);
                    if(number_to_kill > 10)         //如果连续10次都没有收到回应
                    {
                        printf("no ACK get,transmission failed\n");
                        print_time(fp);
                        fprintf(fp,"upload file: %s failed\n",fullname);    //打印错误提示并结束传输
                        break;
                    }
                    int res = sendto(sock,repeat_buf,buf_len,0,(sockaddr*)&addr,sizeof(addr));  //重传上一个数据包
                    rst ++;
                    cout << "resend last blk" << endl;
                    kill_time = 1;
                    while(res != buf_len)
                    {
                        cout << "resend last blk failed: " << kill_time << " times" << endl;
                        if(kill_time <= 10)
                        {
                            res = sendto(sock,repeat_buf,buf_len,0,(sockaddr*)&addr,sizeof(addr));
                            kill_time ++;
                        }
                        else break;
                    }
                    if(kill_time > 10) break;
                    number_to_kill ++;
                }
                if(res > 0)     //收到服务器回应
                {
                    short flag;     //数据包类型
                    memcpy(&flag,buf,2);
                    flag = ntohs(flag);
                    if(flag == 4)       //如果是ACK数据包
                    {
                        short no;       //序列号
                        memcpy(&no,buf + 2,2);      //获取序列号
                        no = ntohs(no);         //网络字节序->主机字节序
                        if(no == block)         //如果ACK序列号是客户端最近发送的数据包的序号
                        {
                            addr = server;
                            if(feof(f) && datalen != 516)       //如果文件全部上传
                            {
                                cout << "upload finished !" << endl;
                                end = clock();
                                run_time = (double)(end - start)/CLOCKS_PER_SEC;        //计算传输时间
                                print_time(fp);
                                printf("average transmission rate: %.2lf kb/s\n",full_size/run_time/1000);
                                fprintf(fp,"upload file: %s finished.resent times: %d;full_size: %d\n",fullname,rst,full_size);
                                break;          //结束传输
                            }
                            //否则，制作下一个数据包
                            block ++;
                            send_data = data_pack(block,f,datalen);
                            buf_len = datalen;
                            full_size += datalen - 4;       //计算文件大小要除去头部
                            number_to_kill = 1;             //重置数据包重发次数
                            memcpy(repeat_buf,send_data,datalen);       //更新repeat缓冲区内容，准备下一轮重传
                            if(send_data == NULL)               //如果构造数据包失败
                            {
                                cout << "file reading mistakes" << endl;
                                break;
                            }
                            //构造成功，将该数据包发出
                            int res = sendto(sock,send_data,datalen,0,(sockaddr*)&addr,sizeof(addr));
                            kill_time = 1;
                            while(res != datalen)       //与sendto重传类似
                            {
                                cout << "send block " << block << " failed: " << kill_time << " times" << endl;
                                if(kill_time <= 10)
                                {
                                    res = sendto(sock,repeat_buf,buf_len,0,(sockaddr*)&addr,sizeof(addr));
                                    kill_time ++;
                                }
                                else break;
                            }
                            if(kill_time > 10) continue;
                            cout << "pack no=" << block << endl;        //向用户输出正在传输的数据包编号
                        }
                    }
                    if(flag == 5)               //ERROR包
                    {
                        short error_code;
                        memcpy(&error_code,buf + 2,2);      //获得错误码
                        error_code = ntohs(error_code);
                        //继续获取详细错误信息
                        char error_str[1024];
                        int iter = 0;
                        while(*(buf + iter + 4) != 0)
                        {
                            memcpy(error_str + iter,buf + iter + 4,1);
                            iter ++;
                        }
                        *(error_str + iter + 1) = '\0';     //我感觉不用加一？？？？？
                        cout << "error " << error_code << " " << error_str << endl;     //输出错误信息
                        print_time(fp);
                        fprintf(fp,"error %d %s\n",error_code,error_str);               //向日志文件输出错误信息
                        break;      //结束传输
                    }
                }
            }
            fclose(f);
        }
        if(choice == 2)         //下载
        {
            addr = get_addr("127.0.0.1",69);    //第一个数据包(RRQ或WRQ)总是发送到69端口，
            printf("input file fullname:\n");
            char fullname[1024];
            scanf("%s",fullname);
            printf("choose a way to upload files:\n");
            printf("1:netascii\n");
            printf("2:octet\n");
            int type;
            scanf("%d",&type);
            if(type == 1) type = 8;     //8对应netascii
            else type = 5;              //5对应octet
            int datalen;
            char *send_data = request_download_pack(fullname,datalen,type);
            buf_len = datalen;
            number_to_kill = 1;
            memcpy(repeat_buf,send_data,datalen);
            int res = sendto(sock,send_data,datalen,0,(sockaddr*)&addr,sizeof(addr));
            start = clock();
            print_time(fp);
            fprintf(fp,"send RRQ for file: %s\n",fullname);
            kill_time = 1;
            while(res != datalen)
            {
                cout << "send RRQ failed: " << kill_time << " times" << endl;
                if(kill_time <= 10)
                {
                    res = sendto(sock,repeat_buf,buf_len,0,(sockaddr*)&addr,sizeof(addr));
                    kill_time ++;
                }
                else break;
            }
            if(kill_time > 10) continue;
            delete[]send_data;
            FILE *f = fopen(fullname,"wb");
            if(f == NULL)
            {
                cout << "file " << fullname << " open failed" << endl;
                continue;
            }
            int want_recv = 1;
            int rst = 0;
            int full_size = 0;
            while(1)        //开始
            {
                char buf[1024];
                sockaddr_in server;
                int len = sizeof(server);
                res = recvfrom(sock,buf,1024,0,(sockaddr*)&server,&len);
                if(res == -1)
                {
                    if(number_to_kill > 10)
                    {
                        printf("no block get,transmission failed\n");
                        print_time(fp);
                        fprintf(fp,"download file: %s failed\n",fullname);
                        break;
                    }
                    int res = sendto(sock,repeat_buf,buf_len,0,(sockaddr*)&addr,sizeof(addr));
                    rst ++;
                    cout << "resend last blk" << endl;
                    kill_time = 1;
                    while(res != buf_len)
                    {
                        cout << "resend last blk failed: " << kill_time << " times" << endl;
                        if(kill_time <= 10)
                        {
                            res = sendto(sock,repeat_buf,buf_len,0,(sockaddr*)&addr,sizeof(addr));
                            kill_time ++;
                        }
                        else break;
                    }
                    if(kill_time > 10) break;
                    number_to_kill ++;
                }
                if(res > 0)
                {
                    short flag;
                    memcpy(&flag,buf,2);
                    flag = ntohs(flag);
                    if(flag == 3)           //如果收到了服务器发来的DATA数据包
                    {
                        addr = server;
                        short no;
                        memcpy(&no,buf + 2,2);
                        no = ntohs(no);
                        cout << "pack no=" << no << endl;       //告诉用户当前收到的数据包的编号
                        char *ack = ack_pack(no);               //对该数据包制作ACK，并不关心其是否为新数据包
                        int send_len = sendto(sock,ack,4,0,(sockaddr*)&addr,sizeof(addr));
                        kill_time = 1;
                        while(send_len != 4)        //发送长度不为4说明ACK包出现错误
                        {
                            cout << "resend last ACK failed: " << kill_time << " times" << endl;
                            if(kill_time <= 10)
                            {
                                send_len = sendto(sock,ack,4,0,(sockaddr*)&addr,sizeof(addr));
                                kill_time ++;
                            }
                            else break;
                        }
                        if(kill_time > 10) break;
                        if(no == want_recv)             //want_recv变量维护用户当前期望收到的下一个数据包编号，如果是
                        {
                            buf_len = 4;
                            number_to_kill = 1;
                            memcpy(repeat_buf,ack,4);   //更新repeat缓冲区，当超时发生时，客户端会不断重传收到的编号最大的数据包的ACK
                            fwrite(buf + 4,res - 4,1,f);
                            full_size += res - 4;
                            if(res - 4 >= 0 && res - 4 < 512)       //如果当前数据包大小小于512bytes，说明传输完成
                            {
                                cout << "download finished!" << endl;
                                end = clock();
                                run_time = (double)(end - start)/CLOCKS_PER_SEC;    //计算时间
                                print_time(fp);
                                printf("average transmission rate: %.2lf kb/s\n",full_size/run_time/1000);
                                fprintf(fp,"download file: %s finished.resent times: %d;full_size: %d\n",fullname,rst,full_size);
                                break;
                            }
                            want_recv ++;
                        }
                    }
                    if(flag == 5)
                    {
                        short error_code;
                        memcpy(&error_code,buf + 2,2);      //获得错误码
                        error_code = ntohs(error_code);
                        //继续获取详细错误信息
                        char error_str[1024];
                        int iter = 0;
                        while(*(buf + iter + 4) != 0)
                        {
                            memcpy(error_str + iter,buf + iter + 4,1);
                            iter ++;
                        }
                        *(error_str + iter + 1) = '\0';     //我感觉不用加一？？？？？
                        cout << "error " << error_code << " " << error_str << endl;     //输出错误信息
                        print_time(fp);
                        fprintf(fp,"error %d %s\n",error_code,error_str);               //向日志文件输出错误信息
                        break;      //结束传输
                    }
                }
            }
            fclose(f);
        }
        if(choice == 0) break;      //关闭客户端
    }
    fclose(fp);
    return 0;
}   
