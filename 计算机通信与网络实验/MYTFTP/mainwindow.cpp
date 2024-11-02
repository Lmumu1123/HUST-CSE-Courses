#pragma execution_character_set("utf-8")
// Qt默认的编码是unicode，不能显示中文的
// 实现了从本地字符集GB到Unicode的转换，用于处理汉语显示乱码等问题

#include "mainwindow.h"      // 包含主窗口的头文件
#include "ui_mainwindow.h"   // 包含UI界面类的生成文件
#define MAX_DATA_SIZE 512    // 定义最大数据大小常量

// 服务器和客户端的IP地址结构
sockaddr_in serverAddress, clientAddress;
// 客户端的socket描述符
SOCKET sock;
// IP地址的长度
unsigned int lenOfAddress;
// 用于非阻塞模式的选项
unsigned long Opt = 1;
// 传输字节、耗时统计
double bytesOfTrans, timeCost;
// 日志文件指针
FILE* logPointer;
char logBuf[MAX_DATA_SIZE];    // 日志缓冲区
time_t initTime;               // 初始时间
tm* info;                      // 时间信息结构体

// 主窗口构造函数
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);  // 设置UI界面
    initUI();           // 初始化界面
}

// 主窗口析构函数
MainWindow::~MainWindow()
{
    delete ui;         // 删除UI对象
}

// 初始化用户界面
void MainWindow::initUI() {
    WSADATA wsaData;
    lenOfAddress = sizeof(struct sockaddr_in);
    // 初始化Winsock
    int nRC = WSAStartup(0x0101, &wsaData);
    if (nRC)
    {
        ui->output->append("初始化错误！");
    }
    if (wsaData.wVersion != 0x0101)  // 检查winsock版本
    {
        ui->output->append("客户端的Winsock版本错误！");
        WSACleanup();
    }
}
void MainWindow::on_PathChoose_pressed() {
    // 文件路径选择
    QDir dir;
    QString PathName = QFileDialog::getOpenFileName(this, tr(""), "", tr("file(*)"));
    //展示路径名称
    ui->PathShow->setText(PathName);
}

// 上传按钮被按下时触发的函数
void MainWindow::on_uploadButton_pressed() {
    // 清空输出窗口
    ui->output->clear();

    // 获取UI上用户输入的文件路径、服务器IP和客户端IP
    QByteArray Qfilename = ui->PathShow->text().toLatin1();
    QByteArray QserverIP = ui->uploadServerIP->text().toLatin1();
    QByteArray QclientIP = ui->uploadLocalIP->text().toLatin1();
    char* filePath = Qfilename.data();
    char* serverIP = QserverIP.data();
    char* clientIP = QclientIP.data();

    // 用于存储处理后的文件名和缓冲区
    char buf[MAX_DATA_SIZE], filename[MAX_DATA_SIZE];
    int temp = 0;

    // 从完整文件路径中提取文件名
    for (int i = 0; filePath[i] != '\0'; i++, temp++) {
        if (filePath[i] == '/') {
            i++;
            temp = 0;
            filename[temp] = filePath[i];
        }
        else {
            filename[temp] = filePath[i];
        }
    }
    filename[temp] = '\0';

    // 打开日志文件以追加模式写入
    logPointer = fopen("tftp.log", "a");
    if (logPointer == NULL) {
        ui->output->append("无法打开日志文件！");
    }

    // 设置服务器地址并使用指定的IP地址和TFTP默认端口69
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(69);  // 将端口号从主机字节顺序转换成网络字节顺序

    serverAddress.sin_addr.S_un.S_addr = inet_addr(serverIP);

    // 设置客户端地址，并使用指定的IP地址和动态分配的端口
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(0);
    clientAddress.sin_addr.S_un.S_addr = inet_addr(clientIP);
    // 创建客户端的UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // 设置socket为非阻塞模式
    ioctlsocket(sock, FIONBIO, &Opt);
    if (sock == INVALID_SOCKET)
    {
        // 创建失败
        sprintf(buf, "客户端socket创建失败！");
        ui->output->append(buf);
        fclose(logPointer);
        return;
    }
    sprintf(buf, "客户端socket创建成功！");
    ui->output->append(buf);

    // 将创建的socket绑定到客户端地址
    bind(sock, (LPSOCKADDR)&clientAddress, sizeof(clientAddress));

    // 初始化用于传输统计的变量
    // 记录时间
    clock_t start, end;
    // 传输的字节
    bytesOfTrans = 0;
    // 用来保存服务器发送数据时的地址信息
    sockaddr_in sender;
    // 等待时间和接收数据包的大小
    int time_wait_ack, receiveSize, chooseMode, resent = 0;

    
    tftpData sendPacket, recievePacket;  // 向服务器发送写请求(WRQ)的包
   // 构造用于发送的WRQ包
    sendPacket.code = htons(CMD_WRQ); 
    chooseMode = ui->uploadMode->currentIndex(); // 写入文件名和传输模式
    sprintf(buf, "chooseMode=%d", chooseMode); 

    ui->output->append(buf);
    if (chooseMode == 0)
        sprintf(sendPacket.filename, "%s%c%s%c", filename, 0, "netascii", 0);
    else
        sprintf(sendPacket.filename, "%s%c%s%c", filename, 0, "octet", 0);    
   
    sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&serverAddress, lenOfAddress);

    // wait for ACK && at most 3s && flash every 20ms
    for (time_wait_ack = 0; time_wait_ack < PKT_RCV_TIMEOUT; time_wait_ack += 20) {

        // 尝试recieve
        receiveSize = recvfrom(sock, (char*)&recievePacket,
            sizeof(tftpData), 0, (struct sockaddr*)&sender, (int*)&lenOfAddress);
        if (receiveSize > 0 && receiveSize < 4) {
            // 查找异常
            sprintf(buf, "收到异常数据包: receiveSize=%d", receiveSize);
            ui->output->append(buf);
        }
        if (receiveSize >= 4 && recievePacket.code == htons(CMD_ACK) && recievePacket.block == htons(0)) {
            // 获得ACK
            break;
        }
        Sleep(20);

        // Sleep 函数用于暂停当前线程的执行指定的毫秒数。
        // 这段时间内，CPU不会处理当前线程的指令，从而允许操作系统将CPU时间分配给其他正在运行的进程或线程。
        // 这一函数通常用于Windows编程，它是 Windows API 的一部分，包含在 <windows.h> 头文件中。
    }
    // WRQ超时
    if (time_wait_ack >= PKT_RCV_TIMEOUT) {
        // 超时
        sprintf(buf, "无法从服务器接收数据。");
        ui->output->append(buf);
        time(&initTime);
        info = localtime(&initTime);
        sprintf(logBuf, "%s 错误：上传 %s，模式：%s，%s\n",
            asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"),
            "无法从服务器接收数据。");
        for (int i = 0; i < MAX_DATA_SIZE; i++) {
            if (logBuf[i] == '\n') {
                logBuf[i] = ' ';
                break;
            }
        }
        fwrite(logBuf, 1, strlen(logBuf), logPointer);
        fclose(logPointer);
        // WRQ错误结束程序
        return;
    }
    // 打开文件
    FILE* fp = NULL;
    if (chooseMode == 0)
        fp = fopen(filePath, "r");
    else
        fp = fopen(filePath, "rb");
    // 该文件不存在
    if (fp == NULL) {
        sprintf(buf, "文件不存在。");
        ui->output->append(buf);
        // 写入日志文件
        // 获取当前时间，并用本地时间格式表示
        time(&initTime);
        info = localtime(&initTime);
        sprintf(logBuf, "%s\n 错误：上传 %s， 模式：%s，%s\n",
            asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"),
            "文件不存在!");
        for (int i = 0; i < MAX_DATA_SIZE; i++) {
            if (logBuf[i] == '\n') {
                logBuf[i] = ' ';
                break;
            }
        }
        fwrite(logBuf, 1, strlen(logBuf), logPointer);
        fclose(logPointer);

        return;
    }
    int sendSize = 0;
    int rxmt;
    unsigned short block = 1;
    // 发送数据
    sendPacket.code = htons(CMD_DATA);

    start = clock(); // 记录当前时间作为传输开始时间
    // 定时循环发送数据，直到发送的数据包大小不是完整的预定大小 DATA_SIZE
    do {
        // 清空数据包缓冲区
        memset(sendPacket.data, 0, sizeof(sendPacket.data));
        // 设置要发送的数据块编号
        sendPacket.block = htons(block);

        // 从文件中读取数据并记录发送大小
        sendSize = fread(sendPacket.data, 1, DATA_SIZE, fp);
        bytesOfTrans += sendSize; // 累计已发送字节

        // 尝试至多 PKT_MAX_RXMT（3次）发送数据并等待ACK
        for (rxmt = 0; rxmt < PKT_MAX_RXMT; rxmt++) {
            // 发送数据包到服务器
            sendto(sock, (char*)&sendPacket, sendSize + 4, 0, (struct sockaddr*)&sender, lenOfAddress);
            sprintf(buf, "发送第 %d 数据块", block);  // 提示发送数据块编号
            ui->output->append(buf);

            // 等待ACK应答，最多等待PKT_RCV_TIMEOUT（3秒）
            for (time_wait_ack = 0; time_wait_ack < PKT_RCV_TIMEOUT; time_wait_ack += 20) {
                // 接收服务器的ACK包
                receiveSize = recvfrom(sock, (char*)&recievePacket,sizeof(tftpData), 0, (struct sockaddr*)&sender, (int*)&lenOfAddress); 
                if (receiveSize > 0 && receiveSize < 4) {
                    sprintf(buf, "收到的数据包异常：receiveSize=%d", receiveSize);
                    ui->output->append(buf);
                }
               // 接收到预期的ACK后跳出等待循环
                if (receiveSize >= 4 && recievePacket.code == htons(CMD_ACK) && recievePacket.block == htons(block)) break;  
                Sleep(20);  // 短暂暂停20毫秒后再次检查
            }
            // 成功发送数据包，准备发送下一个数据包
            if (time_wait_ack < PKT_RCV_TIMEOUT) break;
            else {  // 未收到ACK，打印警告并记录重发
                // 获取当前时间，并用本地时间格式表示
                time(&initTime);
                info = localtime(&initTime);
                sprintf(logBuf, "%s\n 警告：上传 %s， 模式：%s， %s\n",
                    asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"),"未收到ACK，尝试重发");
                for (int i = 0; i < MAX_DATA_SIZE; i++) {
                    if (logBuf[i] == '\n') {
                        logBuf[i] = ' ';
                        break;
                    }
                }
                fwrite(logBuf, 1, strlen(logBuf), logPointer);
                resent++; // 重发计数增加
                continue; // 继续尝试重新发送
            }
        }
        if (rxmt >= PKT_MAX_RXMT) {
            // 超过最大重传次数，结束传输
            sprintf(buf, "无法从服务器接收确认。");
            ui->output->append(buf);
            fclose(fp);  // 关闭文件指针

            // 写入日志文件
            // 获取当前时间，并用本地时间格式表示
            time(&initTime);
            info = localtime(&initTime);
            sprintf(logBuf, "%s\n 错误：上传 %s，模式：%s，等待ACK超时\n",
                asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"));
            for (int i = 0; i < MAX_DATA_SIZE; i++) {
                if (logBuf[i] == '\n') {
                    logBuf[i] = ' ';
                    break;
                }
            }
            fwrite(logBuf, 1, strlen(logBuf), logPointer);
            fclose(logPointer); // 关闭日志文件指针
            return;
        }
        // 数据块编号加一，准备发送下一块数据
        block++;
        // 判断是否是最后一个数据包（不满DATA_SIZE大小）
    } while (sendSize == DATA_SIZE);
    end = clock();   // 记录当前时间作为传输结束时间
    sprintf(buf, "文件发送成功。");
    ui->output->append(buf);   // 输出发送成功的提示信息

    fclose(fp);  // 关闭文件指针
    timeCost = ((double)(end - start)) / CLK_TCK;  // 计算文件传输耗时
    sprintf(buf, "上传文件大小：%.1f kB 耗时：%.2f s", bytesOfTrans / 1024, timeCost);  // 格式化输出文件大小和耗时
    ui->output->append(buf);  // 将文件大小和耗时信息显示到界面上
    sprintf(buf, "上传速度：%.1f kB/s", bytesOfTrans / (1024 * timeCost));  // 格式化输出上传速度
    ui->output->append(buf);  // 将上传速度信息显示到界面上
    sprintf(buf, "重传数据包：%d次, 数据包丢失概率：%.2f%%", resent, 100 * ((double)resent / (resent + block - 1)));   // 计算并格式化输出重传次数和丢包率
    ui->output->append(buf);  // 将重传次数和丢包率信息显示到界面上

    // 获取当前时间，并用本地时间格式表示
    time(&initTime);
    info = localtime(&initTime);

    // 写入日志，记录上传信息
    sprintf(logBuf, "%s\n 信息：上传 %s, 模式：%s, 文件大小：%.1f kB, 耗时：%.2f s\n",
        asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"), bytesOfTrans / 1024, timeCost);
    for (int i = 0; i < MAX_DATA_SIZE; i++) {
        if (logBuf[i] == '\n') {
            logBuf[i] = ' ';   // 将日志信息中的换行符替换为空格
            break;
        }
    }

    fwrite(logBuf, 1, strlen(logBuf), logPointer);  // 将记录的日志信息写入日志文件
    fclose(logPointer);   // 关闭日志文件指针
}

void MainWindow::on_downloadButton_pressed() {
    // 下载按钮被按下时触发的函数
    ui->output->clear();
    QByteArray QServerFile = ui->downloadServerFilename->text().toLatin1();
    QByteArray QlocalFile = ui->downloadLocalFilename->text().toLatin1();
    QByteArray QserverIP = ui->downloadServerIP->text().toLatin1();
    QByteArray QclientIP = ui->downloadLocalIP->text().toLatin1();

    char* localFile = QlocalFile.data();
    char* remoteFile = QServerFile.data();
    char* serverIP = QserverIP.data();
    char* clientIP = QclientIP.data();

    char buf[MAX_DATA_SIZE];
    ui->output->append(remoteFile);
    ui->output->append(localFile);

    logPointer = fopen("tftp.log", "a");
    if (logPointer == NULL) {
        ui->output->append("打开日志文件失败！");
    }


    // 设置服务器地址并使用指定的IP地址和TFTP默认端口69
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(69);
    serverAddress.sin_addr.S_un.S_addr = inet_addr(serverIP);
    // 设置客户端地址，并使用指定的IP地址和动态分配的端口
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(0);
    clientAddress.sin_addr.S_un.S_addr = inet_addr(clientIP);
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // 创建客户端的UDP socket
    ioctlsocket(sock, FIONBIO, &Opt);   // 设置socket为非阻塞模式
    if (sock == INVALID_SOCKET)  // 创建失败
    {
        ui->output->append("客户端socket创建失败！");
        fclose(logPointer);
        return;
    }
    ui->output->append("客户端socket创建成功！");
    // 将创建的socket绑定到客户端地址
    bind(sock, (LPSOCKADDR)&clientAddress, sizeof(clientAddress));

    // 初始化用于传输统计的变量
    // 记录时间
    clock_t start, end;
    // 传输的字节
    bytesOfTrans = 0;
    // 用来保存服务器发送数据时的地址信息
    sockaddr_in sender;
    // 等待时间和接收数据包的大小
    int timeWaitingForData, receiveSize = 0, chooseMode = 0, resent = 0;

    // 向服务器发送写请求(RRQ)的包
    tftpData sendPacket, recievePacket;

    // 设置block初始值为1
    unsigned short block = 1;

    // 发送RRQ
    sendPacket.code = htons(CMD_RRQ);
    // 写入文件名和传输模式
    chooseMode = ui->downloadMode->currentIndex();
    if (chooseMode == 0)
        sprintf(sendPacket.filename, "%s%c%s%c", remoteFile, 0, "netascii", 0);
    else
        sprintf(sendPacket.filename, "%s%c%s%c", remoteFile, 0, "octet", 0);
    // 发送RRQ包
    sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&serverAddress, lenOfAddress);

    // 创建本地文件进行书写
    FILE* fp = NULL;
    if (chooseMode == 0)
        fp = fopen(localFile, "w");
    else
        fp = fopen(localFile, "wb");
    //文件创建失败
    if (fp == NULL) {
        sprintf(buf, "无法创建文件 \"%s\"。", localFile);
        ui->output->append(buf);

        // 获取当前时间，并格式化为本地时间
        time(&initTime);
        info = localtime(&initTime);

        //写入日志文件
        sprintf(logBuf, "%s\n 错误：下载 %s 保存为 %s， 模式: %s, 创建文件 \"%s\" 错误。\n",
            asctime(info), remoteFile, localFile, chooseMode != 0 ? ("octet") : ("netascii"),
            localFile);
        for (int i = 0; i < MAX_DATA_SIZE; i++) {
            if (logBuf[i] == '\n') {
                logBuf[i] = ' ';
                break;
            }
        }
        fwrite(logBuf, 1, strlen(logBuf), logPointer);
        fclose(logPointer);
        return;
    }
    // 获取数据
    start = clock();

    // 准备发送确认数据包 (ACK)
    sendPacket.code = htons(CMD_ACK);

    // 使用 do-while 循环接收数据块和发送确认响应 (ACK)
    do {
        // 定义时间等待变量，用于控制接收等待超时
        for (timeWaitingForData = 0; timeWaitingForData < PKT_RCV_TIMEOUT * PKT_MAX_RXMT; timeWaitingForData += 50) {
            // 尝试接收数据（非阻塞方式）
            receiveSize = recvfrom(sock, (char*)&recievePacket,
                sizeof(tftpData), 0, (struct sockaddr*)&sender,
                (int*)&lenOfAddress);
            // 发送请求 (RRQ) 但未收到响应，判定为连接错误
            if (timeWaitingForData == PKT_RCV_TIMEOUT && block == 1) {
                // 无法从服务器接收数据r
                sprintf(buf, "无法从服务器接收数据。");
                ui->output->append(buf);

                // 获取当前时间，并用本地时间格式表示
                time(&initTime);
                info = localtime(&initTime);

                // 将错误写入日志文件
                sprintf(logBuf, "%s\n 错误：下载 %s 保存为 %s，模式：%s，无法从服务器接收数据。\n",
                    asctime(info), remoteFile, localFile, chooseMode == 0 ? ("netascii") : ("octet"));

                // 替换日志字符串中的换行符为空格
                for (int i = 0; i < MAX_DATA_SIZE; i++) {
                    if (logBuf[i] == '\n') {
                        logBuf[i] = ' ';
                        break;
                    }
                }

                // 写入日志文件并关闭文件
                fwrite(logBuf, 1, strlen(logBuf), logPointer);
                fclose(fp);
                fclose(logPointer);

                // 遇到 RRQ 错误，结束程序
                return;
            }

            // 收到异常小的数据包，记录错误
            if (receiveSize > 0 && receiveSize < 4) {
                sprintf(buf, "收到的数据包异常：receiveSize=%d", receiveSize);
                ui->output->append(buf);
            }
            // 收到正常的数据包
            else if (receiveSize >= 4 && recievePacket.code == htons(CMD_DATA) && recievePacket.block == htons(block)) {
                // 打印收到的数据包信息
                sprintf(buf, "数据：数据块=%d, 数据大小=%d\n", ntohs(recievePacket.block), receiveSize - 4);
                ui->output->append(buf);

                // 发送 ACK 作为对数据包的响应
                sendPacket.block = recievePacket.block;
                sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&sender, lenOfAddress);

                // 将接收到的实际数据写入文件
                fwrite(recievePacket.data, 1, receiveSize - 4, fp);

                // 跳出 for 循环，准备接收下一个数据块
                break;
            }

            // 若在指定时间内未收到数据，则需要重发 ACK
            else {
                if (timeWaitingForData != 0 && timeWaitingForData % PKT_RCV_TIMEOUT == 0) {
                    // 重发 ACK 数据包
                    sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&sender, lenOfAddress);
                    // 写入日志，提示重发
                    time(&initTime);
                    info = localtime(&initTime);
                    sprintf(logBuf, "%s\n 警告：下载 %s 保存为 %s,  模式：%s, 未能接收到数据块 #%d, 重发\n",
                        asctime(info), remoteFile, localFile, chooseMode == 0 ? ("netascii") : ("octet"),
                        block);
                    for (int i = 0; i < MAX_DATA_SIZE; i++) {
                        if (logBuf[i] == '\n') {
                            logBuf[i] = ' ';
                            break;
                        }
                    }
                    fwrite(logBuf, 1, strlen(logBuf), logPointer);
                    resent++; // 重发计数增加
                }
            }
            // 等待 50 毫秒再次尝试接收
            Sleep(50);
        }

        // 更新文件传输总字节数
        bytesOfTrans += (receiveSize - 4);

        // 超时检测
        if (timeWaitingForData >= PKT_RCV_TIMEOUT * PKT_MAX_RXMT) {
            // 超时等待数据块
            sprintf(buf, "等待数据块 #%d 超时。\n", block);
            ui->output->append(buf);

            // 关闭文件和日志
            fclose(fp);
            time(&initTime);
            info = localtime(&initTime);

            // 记录错误日志并退出
            sprintf(logBuf, "%s\n 错误：下载 %s 保存为 %s, 模式：%s, 等待数据块 #%d 超时。\n",
                asctime(info), remoteFile, localFile, chooseMode == 0 ? ("netascii") : ("octet"),
                block);
            for (int i = 0; i < MAX_DATA_SIZE; i++) {
                if (logBuf[i] == '\n') {
                    logBuf[i] = ' ';
                    break;
                }
            }
            fwrite(logBuf, 1, strlen(logBuf), logPointer);
            fclose(logPointer);
            return;
        }
        // 准备接收下一个数据块
        block++;
        // 循环直到收到小于预期大小的数据包，意味着文件传输结束
    } while (receiveSize == DATA_SIZE + 4);

    // 记录结束时间并计算下载耗时
    end = clock();
    timeCost = ((double)(end - start)) / CLK_TCK;

    // 输出文件下载大小和耗时信息
    sprintf(buf, "下载文件大小：%.1f kB 耗时：%.2f s", bytesOfTrans / 1024, timeCost);
    ui->output->append(buf);  // 将文件大小和耗时信息显示到界面上

    // 计算并输出下载速度
    sprintf(buf, "下载速度：%.1f kB/s", bytesOfTrans / (1024 * timeCost));
    ui->output->append(buf);   // 将下载速度信息显示到界面上

    // 计算并输出重发包数和丢包率
    sprintf(buf, "重传数据包：%d次, 数据包丢失概率：%.2f%%", resent, 100 * ((double)resent / (resent + block - 1)));
    ui->output->append(buf);   // 将重传次数和丢包率信息显示到界面上

    // 关闭文件读取
    fclose(fp);

    // 获取当前时间，并格式化为本地时间
    time(&initTime);
    info = localtime(&initTime);

    // 输出下载完成的信息日志，包括下载的文件信息、模式、下载的大小、耗时和下载速度
    sprintf(logBuf, "%s\n 信息：下载 %s 保存为 %s，模式：%s，文件大小：%.1f kB，耗时：%.2f s，下载速度：%.1f kB/s\n",
        asctime(info), remoteFile, localFile, chooseMode == 0 ? ("netascii") : ("octet"),
        bytesOfTrans / 1024, timeCost, bytesOfTrans / (1024 * timeCost));

    // 下面的代码用于去除日志中的换行符，因为一些日志系统可能不支持换行符。
    for (int i = 0; i < MAX_DATA_SIZE; i++) {
        if (logBuf[i] == '\n') {
            logBuf[i] = ' ';    // 将日志信息中的换行符替换为空格
            break;
        }
    }

    // 将完成日志写入日志文件，并关闭日志文件指针
    fwrite(logBuf, 1, strlen(logBuf), logPointer);
    fclose(logPointer);
}
