#pragma execution_character_set("utf-8")
// QtĬ�ϵı�����unicode��������ʾ���ĵ�
// ʵ���˴ӱ����ַ���GB��Unicode��ת�������ڴ�������ʾ���������

#include "mainwindow.h"      // ���������ڵ�ͷ�ļ�
#include "ui_mainwindow.h"   // ����UI������������ļ�
#define MAX_DATA_SIZE 512    // ����������ݴ�С����

// �������Ϳͻ��˵�IP��ַ�ṹ
sockaddr_in serverAddress, clientAddress;
// �ͻ��˵�socket������
SOCKET sock;
// IP��ַ�ĳ���
unsigned int lenOfAddress;
// ���ڷ�����ģʽ��ѡ��
unsigned long Opt = 1;
// �����ֽڡ���ʱͳ��
double bytesOfTrans, timeCost;
// ��־�ļ�ָ��
FILE* logPointer;
char logBuf[MAX_DATA_SIZE];    // ��־������
time_t initTime;               // ��ʼʱ��
tm* info;                      // ʱ����Ϣ�ṹ��

// �����ڹ��캯��
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);  // ����UI����
    initUI();           // ��ʼ������
}

// ��������������
MainWindow::~MainWindow()
{
    delete ui;         // ɾ��UI����
}

// ��ʼ���û�����
void MainWindow::initUI() {
    WSADATA wsaData;
    lenOfAddress = sizeof(struct sockaddr_in);
    // ��ʼ��Winsock
    int nRC = WSAStartup(0x0101, &wsaData);
    if (nRC)
    {
        ui->output->append("��ʼ������");
    }
    if (wsaData.wVersion != 0x0101)  // ���winsock�汾
    {
        ui->output->append("�ͻ��˵�Winsock�汾����");
        WSACleanup();
    }
}
void MainWindow::on_PathChoose_pressed() {
    // �ļ�·��ѡ��
    QDir dir;
    QString PathName = QFileDialog::getOpenFileName(this, tr(""), "", tr("file(*)"));
    //չʾ·������
    ui->PathShow->setText(PathName);
}

// �ϴ���ť������ʱ�����ĺ���
void MainWindow::on_uploadButton_pressed() {
    // ����������
    ui->output->clear();

    // ��ȡUI���û�������ļ�·����������IP�Ϳͻ���IP
    QByteArray Qfilename = ui->PathShow->text().toLatin1();
    QByteArray QserverIP = ui->uploadServerIP->text().toLatin1();
    QByteArray QclientIP = ui->uploadLocalIP->text().toLatin1();
    char* filePath = Qfilename.data();
    char* serverIP = QserverIP.data();
    char* clientIP = QclientIP.data();

    // ���ڴ洢�������ļ����ͻ�����
    char buf[MAX_DATA_SIZE], filename[MAX_DATA_SIZE];
    int temp = 0;

    // �������ļ�·������ȡ�ļ���
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

    // ����־�ļ���׷��ģʽд��
    logPointer = fopen("tftp.log", "a");
    if (logPointer == NULL) {
        ui->output->append("�޷�����־�ļ���");
    }

    // ���÷�������ַ��ʹ��ָ����IP��ַ��TFTPĬ�϶˿�69
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(69);  // ���˿ںŴ������ֽ�˳��ת���������ֽ�˳��

    serverAddress.sin_addr.S_un.S_addr = inet_addr(serverIP);

    // ���ÿͻ��˵�ַ����ʹ��ָ����IP��ַ�Ͷ�̬����Ķ˿�
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(0);
    clientAddress.sin_addr.S_un.S_addr = inet_addr(clientIP);
    // �����ͻ��˵�UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // ����socketΪ������ģʽ
    ioctlsocket(sock, FIONBIO, &Opt);
    if (sock == INVALID_SOCKET)
    {
        // ����ʧ��
        sprintf(buf, "�ͻ���socket����ʧ�ܣ�");
        ui->output->append(buf);
        fclose(logPointer);
        return;
    }
    sprintf(buf, "�ͻ���socket�����ɹ���");
    ui->output->append(buf);

    // ��������socket�󶨵��ͻ��˵�ַ
    bind(sock, (LPSOCKADDR)&clientAddress, sizeof(clientAddress));

    // ��ʼ�����ڴ���ͳ�Ƶı���
    // ��¼ʱ��
    clock_t start, end;
    // ������ֽ�
    bytesOfTrans = 0;
    // ���������������������ʱ�ĵ�ַ��Ϣ
    sockaddr_in sender;
    // �ȴ�ʱ��ͽ������ݰ��Ĵ�С
    int time_wait_ack, receiveSize, chooseMode, resent = 0;

    
    tftpData sendPacket, recievePacket;  // �����������д����(WRQ)�İ�
   // �������ڷ��͵�WRQ��
    sendPacket.code = htons(CMD_WRQ); 
    chooseMode = ui->uploadMode->currentIndex(); // д���ļ����ʹ���ģʽ
    sprintf(buf, "chooseMode=%d", chooseMode); 

    ui->output->append(buf);
    if (chooseMode == 0)
        sprintf(sendPacket.filename, "%s%c%s%c", filename, 0, "netascii", 0);
    else
        sprintf(sendPacket.filename, "%s%c%s%c", filename, 0, "octet", 0);    
   
    sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&serverAddress, lenOfAddress);

    // wait for ACK && at most 3s && flash every 20ms
    for (time_wait_ack = 0; time_wait_ack < PKT_RCV_TIMEOUT; time_wait_ack += 20) {

        // ����recieve
        receiveSize = recvfrom(sock, (char*)&recievePacket,
            sizeof(tftpData), 0, (struct sockaddr*)&sender, (int*)&lenOfAddress);
        if (receiveSize > 0 && receiveSize < 4) {
            // �����쳣
            sprintf(buf, "�յ��쳣���ݰ�: receiveSize=%d", receiveSize);
            ui->output->append(buf);
        }
        if (receiveSize >= 4 && recievePacket.code == htons(CMD_ACK) && recievePacket.block == htons(0)) {
            // ���ACK
            break;
        }
        Sleep(20);

        // Sleep ����������ͣ��ǰ�̵߳�ִ��ָ���ĺ�������
        // ���ʱ���ڣ�CPU���ᴦ��ǰ�̵߳�ָ��Ӷ��������ϵͳ��CPUʱ�����������������еĽ��̻��̡߳�
        // ��һ����ͨ������Windows��̣����� Windows API ��һ���֣������� <windows.h> ͷ�ļ��С�
    }
    // WRQ��ʱ
    if (time_wait_ack >= PKT_RCV_TIMEOUT) {
        // ��ʱ
        sprintf(buf, "�޷��ӷ������������ݡ�");
        ui->output->append(buf);
        time(&initTime);
        info = localtime(&initTime);
        sprintf(logBuf, "%s �����ϴ� %s��ģʽ��%s��%s\n",
            asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"),
            "�޷��ӷ������������ݡ�");
        for (int i = 0; i < MAX_DATA_SIZE; i++) {
            if (logBuf[i] == '\n') {
                logBuf[i] = ' ';
                break;
            }
        }
        fwrite(logBuf, 1, strlen(logBuf), logPointer);
        fclose(logPointer);
        // WRQ�����������
        return;
    }
    // ���ļ�
    FILE* fp = NULL;
    if (chooseMode == 0)
        fp = fopen(filePath, "r");
    else
        fp = fopen(filePath, "rb");
    // ���ļ�������
    if (fp == NULL) {
        sprintf(buf, "�ļ������ڡ�");
        ui->output->append(buf);
        // д����־�ļ�
        // ��ȡ��ǰʱ�䣬���ñ���ʱ���ʽ��ʾ
        time(&initTime);
        info = localtime(&initTime);
        sprintf(logBuf, "%s\n �����ϴ� %s�� ģʽ��%s��%s\n",
            asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"),
            "�ļ�������!");
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
    // ��������
    sendPacket.code = htons(CMD_DATA);

    start = clock(); // ��¼��ǰʱ����Ϊ���俪ʼʱ��
    // ��ʱѭ���������ݣ�ֱ�����͵����ݰ���С����������Ԥ����С DATA_SIZE
    do {
        // ������ݰ�������
        memset(sendPacket.data, 0, sizeof(sendPacket.data));
        // ����Ҫ���͵����ݿ���
        sendPacket.block = htons(block);

        // ���ļ��ж�ȡ���ݲ���¼���ʹ�С
        sendSize = fread(sendPacket.data, 1, DATA_SIZE, fp);
        bytesOfTrans += sendSize; // �ۼ��ѷ����ֽ�

        // �������� PKT_MAX_RXMT��3�Σ��������ݲ��ȴ�ACK
        for (rxmt = 0; rxmt < PKT_MAX_RXMT; rxmt++) {
            // �������ݰ���������
            sendto(sock, (char*)&sendPacket, sendSize + 4, 0, (struct sockaddr*)&sender, lenOfAddress);
            sprintf(buf, "���͵� %d ���ݿ�", block);  // ��ʾ�������ݿ���
            ui->output->append(buf);

            // �ȴ�ACKӦ�����ȴ�PKT_RCV_TIMEOUT��3�룩
            for (time_wait_ack = 0; time_wait_ack < PKT_RCV_TIMEOUT; time_wait_ack += 20) {
                // ���շ�������ACK��
                receiveSize = recvfrom(sock, (char*)&recievePacket,sizeof(tftpData), 0, (struct sockaddr*)&sender, (int*)&lenOfAddress); 
                if (receiveSize > 0 && receiveSize < 4) {
                    sprintf(buf, "�յ������ݰ��쳣��receiveSize=%d", receiveSize);
                    ui->output->append(buf);
                }
               // ���յ�Ԥ�ڵ�ACK�������ȴ�ѭ��
                if (receiveSize >= 4 && recievePacket.code == htons(CMD_ACK) && recievePacket.block == htons(block)) break;  
                Sleep(20);  // ������ͣ20������ٴμ��
            }
            // �ɹ��������ݰ���׼��������һ�����ݰ�
            if (time_wait_ack < PKT_RCV_TIMEOUT) break;
            else {  // δ�յ�ACK����ӡ���沢��¼�ط�
                // ��ȡ��ǰʱ�䣬���ñ���ʱ���ʽ��ʾ
                time(&initTime);
                info = localtime(&initTime);
                sprintf(logBuf, "%s\n ���棺�ϴ� %s�� ģʽ��%s�� %s\n",
                    asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"),"δ�յ�ACK�������ط�");
                for (int i = 0; i < MAX_DATA_SIZE; i++) {
                    if (logBuf[i] == '\n') {
                        logBuf[i] = ' ';
                        break;
                    }
                }
                fwrite(logBuf, 1, strlen(logBuf), logPointer);
                resent++; // �ط���������
                continue; // �����������·���
            }
        }
        if (rxmt >= PKT_MAX_RXMT) {
            // ��������ش���������������
            sprintf(buf, "�޷��ӷ���������ȷ�ϡ�");
            ui->output->append(buf);
            fclose(fp);  // �ر��ļ�ָ��

            // д����־�ļ�
            // ��ȡ��ǰʱ�䣬���ñ���ʱ���ʽ��ʾ
            time(&initTime);
            info = localtime(&initTime);
            sprintf(logBuf, "%s\n �����ϴ� %s��ģʽ��%s���ȴ�ACK��ʱ\n",
                asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"));
            for (int i = 0; i < MAX_DATA_SIZE; i++) {
                if (logBuf[i] == '\n') {
                    logBuf[i] = ' ';
                    break;
                }
            }
            fwrite(logBuf, 1, strlen(logBuf), logPointer);
            fclose(logPointer); // �ر���־�ļ�ָ��
            return;
        }
        // ���ݿ��ż�һ��׼��������һ������
        block++;
        // �ж��Ƿ������һ�����ݰ�������DATA_SIZE��С��
    } while (sendSize == DATA_SIZE);
    end = clock();   // ��¼��ǰʱ����Ϊ�������ʱ��
    sprintf(buf, "�ļ����ͳɹ���");
    ui->output->append(buf);   // ������ͳɹ�����ʾ��Ϣ

    fclose(fp);  // �ر��ļ�ָ��
    timeCost = ((double)(end - start)) / CLK_TCK;  // �����ļ������ʱ
    sprintf(buf, "�ϴ��ļ���С��%.1f kB ��ʱ��%.2f s", bytesOfTrans / 1024, timeCost);  // ��ʽ������ļ���С�ͺ�ʱ
    ui->output->append(buf);  // ���ļ���С�ͺ�ʱ��Ϣ��ʾ��������
    sprintf(buf, "�ϴ��ٶȣ�%.1f kB/s", bytesOfTrans / (1024 * timeCost));  // ��ʽ������ϴ��ٶ�
    ui->output->append(buf);  // ���ϴ��ٶ���Ϣ��ʾ��������
    sprintf(buf, "�ش����ݰ���%d��, ���ݰ���ʧ���ʣ�%.2f%%", resent, 100 * ((double)resent / (resent + block - 1)));   // ���㲢��ʽ������ش������Ͷ�����
    ui->output->append(buf);  // ���ش������Ͷ�������Ϣ��ʾ��������

    // ��ȡ��ǰʱ�䣬���ñ���ʱ���ʽ��ʾ
    time(&initTime);
    info = localtime(&initTime);

    // д����־����¼�ϴ���Ϣ
    sprintf(logBuf, "%s\n ��Ϣ���ϴ� %s, ģʽ��%s, �ļ���С��%.1f kB, ��ʱ��%.2f s\n",
        asctime(info), filename, chooseMode == 0 ? ("netascii") : ("octet"), bytesOfTrans / 1024, timeCost);
    for (int i = 0; i < MAX_DATA_SIZE; i++) {
        if (logBuf[i] == '\n') {
            logBuf[i] = ' ';   // ����־��Ϣ�еĻ��з��滻Ϊ�ո�
            break;
        }
    }

    fwrite(logBuf, 1, strlen(logBuf), logPointer);  // ����¼����־��Ϣд����־�ļ�
    fclose(logPointer);   // �ر���־�ļ�ָ��
}

void MainWindow::on_downloadButton_pressed() {
    // ���ذ�ť������ʱ�����ĺ���
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
        ui->output->append("����־�ļ�ʧ�ܣ�");
    }


    // ���÷�������ַ��ʹ��ָ����IP��ַ��TFTPĬ�϶˿�69
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(69);
    serverAddress.sin_addr.S_un.S_addr = inet_addr(serverIP);
    // ���ÿͻ��˵�ַ����ʹ��ָ����IP��ַ�Ͷ�̬����Ķ˿�
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(0);
    clientAddress.sin_addr.S_un.S_addr = inet_addr(clientIP);
    
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // �����ͻ��˵�UDP socket
    ioctlsocket(sock, FIONBIO, &Opt);   // ����socketΪ������ģʽ
    if (sock == INVALID_SOCKET)  // ����ʧ��
    {
        ui->output->append("�ͻ���socket����ʧ�ܣ�");
        fclose(logPointer);
        return;
    }
    ui->output->append("�ͻ���socket�����ɹ���");
    // ��������socket�󶨵��ͻ��˵�ַ
    bind(sock, (LPSOCKADDR)&clientAddress, sizeof(clientAddress));

    // ��ʼ�����ڴ���ͳ�Ƶı���
    // ��¼ʱ��
    clock_t start, end;
    // ������ֽ�
    bytesOfTrans = 0;
    // ���������������������ʱ�ĵ�ַ��Ϣ
    sockaddr_in sender;
    // �ȴ�ʱ��ͽ������ݰ��Ĵ�С
    int timeWaitingForData, receiveSize = 0, chooseMode = 0, resent = 0;

    // �����������д����(RRQ)�İ�
    tftpData sendPacket, recievePacket;

    // ����block��ʼֵΪ1
    unsigned short block = 1;

    // ����RRQ
    sendPacket.code = htons(CMD_RRQ);
    // д���ļ����ʹ���ģʽ
    chooseMode = ui->downloadMode->currentIndex();
    if (chooseMode == 0)
        sprintf(sendPacket.filename, "%s%c%s%c", remoteFile, 0, "netascii", 0);
    else
        sprintf(sendPacket.filename, "%s%c%s%c", remoteFile, 0, "octet", 0);
    // ����RRQ��
    sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&serverAddress, lenOfAddress);

    // ���������ļ�������д
    FILE* fp = NULL;
    if (chooseMode == 0)
        fp = fopen(localFile, "w");
    else
        fp = fopen(localFile, "wb");
    //�ļ�����ʧ��
    if (fp == NULL) {
        sprintf(buf, "�޷������ļ� \"%s\"��", localFile);
        ui->output->append(buf);

        // ��ȡ��ǰʱ�䣬����ʽ��Ϊ����ʱ��
        time(&initTime);
        info = localtime(&initTime);

        //д����־�ļ�
        sprintf(logBuf, "%s\n �������� %s ����Ϊ %s�� ģʽ: %s, �����ļ� \"%s\" ����\n",
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
    // ��ȡ����
    start = clock();

    // ׼������ȷ�����ݰ� (ACK)
    sendPacket.code = htons(CMD_ACK);

    // ʹ�� do-while ѭ���������ݿ�ͷ���ȷ����Ӧ (ACK)
    do {
        // ����ʱ��ȴ����������ڿ��ƽ��յȴ���ʱ
        for (timeWaitingForData = 0; timeWaitingForData < PKT_RCV_TIMEOUT * PKT_MAX_RXMT; timeWaitingForData += 50) {
            // ���Խ������ݣ���������ʽ��
            receiveSize = recvfrom(sock, (char*)&recievePacket,
                sizeof(tftpData), 0, (struct sockaddr*)&sender,
                (int*)&lenOfAddress);
            // �������� (RRQ) ��δ�յ���Ӧ���ж�Ϊ���Ӵ���
            if (timeWaitingForData == PKT_RCV_TIMEOUT && block == 1) {
                // �޷��ӷ�������������r
                sprintf(buf, "�޷��ӷ������������ݡ�");
                ui->output->append(buf);

                // ��ȡ��ǰʱ�䣬���ñ���ʱ���ʽ��ʾ
                time(&initTime);
                info = localtime(&initTime);

                // ������д����־�ļ�
                sprintf(logBuf, "%s\n �������� %s ����Ϊ %s��ģʽ��%s���޷��ӷ������������ݡ�\n",
                    asctime(info), remoteFile, localFile, chooseMode == 0 ? ("netascii") : ("octet"));

                // �滻��־�ַ����еĻ��з�Ϊ�ո�
                for (int i = 0; i < MAX_DATA_SIZE; i++) {
                    if (logBuf[i] == '\n') {
                        logBuf[i] = ' ';
                        break;
                    }
                }

                // д����־�ļ����ر��ļ�
                fwrite(logBuf, 1, strlen(logBuf), logPointer);
                fclose(fp);
                fclose(logPointer);

                // ���� RRQ ���󣬽�������
                return;
            }

            // �յ��쳣С�����ݰ�����¼����
            if (receiveSize > 0 && receiveSize < 4) {
                sprintf(buf, "�յ������ݰ��쳣��receiveSize=%d", receiveSize);
                ui->output->append(buf);
            }
            // �յ����������ݰ�
            else if (receiveSize >= 4 && recievePacket.code == htons(CMD_DATA) && recievePacket.block == htons(block)) {
                // ��ӡ�յ������ݰ���Ϣ
                sprintf(buf, "���ݣ����ݿ�=%d, ���ݴ�С=%d\n", ntohs(recievePacket.block), receiveSize - 4);
                ui->output->append(buf);

                // ���� ACK ��Ϊ�����ݰ�����Ӧ
                sendPacket.block = recievePacket.block;
                sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&sender, lenOfAddress);

                // �����յ���ʵ������д���ļ�
                fwrite(recievePacket.data, 1, receiveSize - 4, fp);

                // ���� for ѭ����׼��������һ�����ݿ�
                break;
            }

            // ����ָ��ʱ����δ�յ����ݣ�����Ҫ�ط� ACK
            else {
                if (timeWaitingForData != 0 && timeWaitingForData % PKT_RCV_TIMEOUT == 0) {
                    // �ط� ACK ���ݰ�
                    sendto(sock, (char*)&sendPacket, sizeof(tftpData), 0, (struct sockaddr*)&sender, lenOfAddress);
                    // д����־����ʾ�ط�
                    time(&initTime);
                    info = localtime(&initTime);
                    sprintf(logBuf, "%s\n ���棺���� %s ����Ϊ %s,  ģʽ��%s, δ�ܽ��յ����ݿ� #%d, �ط�\n",
                        asctime(info), remoteFile, localFile, chooseMode == 0 ? ("netascii") : ("octet"),
                        block);
                    for (int i = 0; i < MAX_DATA_SIZE; i++) {
                        if (logBuf[i] == '\n') {
                            logBuf[i] = ' ';
                            break;
                        }
                    }
                    fwrite(logBuf, 1, strlen(logBuf), logPointer);
                    resent++; // �ط���������
                }
            }
            // �ȴ� 50 �����ٴγ��Խ���
            Sleep(50);
        }

        // �����ļ��������ֽ���
        bytesOfTrans += (receiveSize - 4);

        // ��ʱ���
        if (timeWaitingForData >= PKT_RCV_TIMEOUT * PKT_MAX_RXMT) {
            // ��ʱ�ȴ����ݿ�
            sprintf(buf, "�ȴ����ݿ� #%d ��ʱ��\n", block);
            ui->output->append(buf);

            // �ر��ļ�����־
            fclose(fp);
            time(&initTime);
            info = localtime(&initTime);

            // ��¼������־���˳�
            sprintf(logBuf, "%s\n �������� %s ����Ϊ %s, ģʽ��%s, �ȴ����ݿ� #%d ��ʱ��\n",
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
        // ׼��������һ�����ݿ�
        block++;
        // ѭ��ֱ���յ�С��Ԥ�ڴ�С�����ݰ�����ζ���ļ��������
    } while (receiveSize == DATA_SIZE + 4);

    // ��¼����ʱ�䲢�������غ�ʱ
    end = clock();
    timeCost = ((double)(end - start)) / CLK_TCK;

    // ����ļ����ش�С�ͺ�ʱ��Ϣ
    sprintf(buf, "�����ļ���С��%.1f kB ��ʱ��%.2f s", bytesOfTrans / 1024, timeCost);
    ui->output->append(buf);  // ���ļ���С�ͺ�ʱ��Ϣ��ʾ��������

    // ���㲢��������ٶ�
    sprintf(buf, "�����ٶȣ�%.1f kB/s", bytesOfTrans / (1024 * timeCost));
    ui->output->append(buf);   // �������ٶ���Ϣ��ʾ��������

    // ���㲢����ط������Ͷ�����
    sprintf(buf, "�ش����ݰ���%d��, ���ݰ���ʧ���ʣ�%.2f%%", resent, 100 * ((double)resent / (resent + block - 1)));
    ui->output->append(buf);   // ���ش������Ͷ�������Ϣ��ʾ��������

    // �ر��ļ���ȡ
    fclose(fp);

    // ��ȡ��ǰʱ�䣬����ʽ��Ϊ����ʱ��
    time(&initTime);
    info = localtime(&initTime);

    // ���������ɵ���Ϣ��־���������ص��ļ���Ϣ��ģʽ�����صĴ�С����ʱ�������ٶ�
    sprintf(logBuf, "%s\n ��Ϣ������ %s ����Ϊ %s��ģʽ��%s���ļ���С��%.1f kB����ʱ��%.2f s�������ٶȣ�%.1f kB/s\n",
        asctime(info), remoteFile, localFile, chooseMode == 0 ? ("netascii") : ("octet"),
        bytesOfTrans / 1024, timeCost, bytesOfTrans / (1024 * timeCost));

    // ����Ĵ�������ȥ����־�еĻ��з�����ΪһЩ��־ϵͳ���ܲ�֧�ֻ��з���
    for (int i = 0; i < MAX_DATA_SIZE; i++) {
        if (logBuf[i] == '\n') {
            logBuf[i] = ' ';    // ����־��Ϣ�еĻ��з��滻Ϊ�ո�
            break;
        }
    }

    // �������־д����־�ļ������ر���־�ļ�ָ��
    fwrite(logBuf, 1, strlen(logBuf), logPointer);
    fclose(logPointer);
}
