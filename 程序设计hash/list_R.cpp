#include "tree.h"

/**
 * @brief ת�� FILETIME ʱ���ʽ���Ա�׼��ʽ������ļ�
 * @param fileTime ��ת���� FILETIME �ṹ��
 * @param outfile �������������д��ת�����ʱ��
 */
void PrintFileTime(const FILETIME& fileTime, ofstream& outfile) {
    SYSTEMTIME sysTime;
    FILETIME localFileTime;

    FileTimeToLocalFileTime(&fileTime, &localFileTime); // ���ļ�ʱ��ת��Ϊ�����ļ�ʱ��
    FileTimeToSystemTime(&localFileTime, &sysTime); // �������ļ�ʱ��ת��Ϊϵͳʱ��

    //cout <<  sysTime.wYear << "��" << sysTime.wMonth << "��" << sysTime.wDay<<"��";
    //cout << sysTime.wHour << ": " << sysTime.wMinute << ": " << sysTime.wSecond  << endl;

    outfile << sysTime.wYear << "��" << sysTime.wMonth << "��" << sysTime.wDay << "�� ";
    outfile << sysTime.wHour << ":" << sysTime.wMinute << ":" << sysTime.wSecond << endl;

}

/**
 * @brief �г�ָ��Ŀ¼�ڵ��µ������ļ�����ͳ�������Ϣд������ļ�
 * @param p ָ��Ŀ¼�����ڵ�
 * @param outputFileName �涨������ļ���
 * @note �˺�������Ŀ¼���ļ�����������С���Լ�����������ļ��������Ϣ
 */
void ListFiles(TreeNode* p, const string& outputFileName) {
    int fileCount = 0;    // �ļ�����
    int totalSize = 0;    // �ļ��ܴ�С
    time_t MAX_time = 0;  // ������ļ�ʱ��
    time_t MIN_time = LONG_TIME;    // ������ļ�ʱ��
    TreeNode* MAX_node = (TreeNode*)malloc(sizeof(TreeNode)); // ������ļ��ڵ�
    TreeNode* MIN_node = (TreeNode*)malloc(sizeof(TreeNode)); // ������ļ��ڵ�

    // ����Ŀ¼�ڵ���ӽڵ�
    TreeNode* node = p->child;       
    while (node) {
        if (node->isDirectory == 0) { // ������ļ�
            fileCount++;
            totalSize = totalSize + node->size; // �ۼ��ļ���С
            // ��������������ļ�����Ϣ
            if (node->time > MAX_time) {
                MAX_time = node->time;
                MAX_node = node;
            }
            if (node->time < MIN_time) {
                MIN_time = node->time;
                MIN_node = node;
            }
        }
        // �ƶ�����һ���ֵܽڵ�
        node = node->sibling;
    }
    //char string_time_max[20] = {};
    //char sting_time_min[20] = {};
    //printf("%d %d %s %s\n",fileCount,totalSize, Microsecond_To_DateTime(MAX_time, string_time_max), Microsecond_To_DateTime(MIN_time, sting_time_min));
    
    // ������ļ���׼��д��ͳ����Ϣ
    ofstream outfile(outputFileName, ios::app);
    // д�����ļ��������ļ���С
    outfile << "���ļ���: " << fileCount << "\n���ļ���С: " << totalSize << "B" << endl;
    // ������ļ���д�������������ļ���Ϣ
    if (fileCount != 0) {
        outfile << "�����ļ�: " << MAX_node->name << "\n�����ļ���С: " << MAX_node->size << "B\n����ʱ��: ";
        PrintFileTime(MAX_node->file_time, outfile);
        outfile << "�����ļ�: " << MIN_node->name << "\n�����ļ���С: " << MIN_node->size << "B\n����ʱ��: ";
        PrintFileTime(MIN_node->file_time, outfile);
    }
    else {
        outfile << "�����ļ�: ��\n�����ļ���С: ��\n����ʱ��: ��" << endl;
        outfile << "�����ļ�: ��\n�����ļ���С: ��\n����ʱ��: �� " << endl;

    }
    outfile << endl;
    // ������ر�����ļ�
    outfile.close();
}

/**
 * @brief ���������ļ���ָ����Ŀ¼������Ŀ¼�µ��ļ���Ϣ�����ָ���ļ�
 * @param inputFileName �����ļ��������а���Ҫͳ�Ƶ�Ŀ¼�б�
 * @param outputFileName ����ļ������������ͳ�ƽ��
 * @note �����ļ��е�ÿһ��ӦΪһ��Ŀ¼��·�����ú�����ͳ��Ŀ¼�µ��ļ���Ϣ
 */
void ProcessDirectories(const string& inputFileName, const string& outputFileName) {
    ifstream infile(inputFileName);
    ofstream outfile(outputFileName);

    // ����ļ��Ƿ�ɹ���
    if (!infile.is_open() || !outfile.is_open()) {
        cerr << "Error opening input or output file." << endl;
        return;
    }

    bool is_reading_dirs = false; // ��ʶ�Ƿ����ڶ�ȡĿ¼�б�
    string line;
    int num = 0;  // ����������¼��ǰ������ǵڼ���Ŀ¼

    // ���ж�ȡ�����ļ�
    while (getline(infile, line)) {
        //cout << "��ǰ�е�����Ϊ��" << line << endl;

        if (line == "stat dirs") {
            is_reading_dirs = true; // ��ʼ��ȡĿ¼�б�
            continue;
        }
        else if (line == "end of dirs") {
            is_reading_dirs = false; // ������ȡĿ¼�б�
            break;
        }
        // ������ڶ�ȡĿ¼�б�
        if (is_reading_dirs) {
            // ����Ŀ¼��Ϣ
            if (!line.empty() && line.back() == '\\') {
                // ȥ��·������β��'\'
                line.pop_back();

                num++;
                // ����·���ҵ���Ӧ��Ŀ¼�ڵ�
                TreeNode* node = findNodeByPath(line);
                if (!node) { // ����Ҳ����ڵ㣬�����������Ϣ���ļ�
                    ofstream outfile(outputFileName, ios::app);
                    outfile << "# " << num << " #  " << line << ":\nError - Unable to open directory\n\n";
                    outfile.close();
                }
                else {  // ����ҵ��ڵ㣬���Ŀ¼�����ļ���Ϣ
                    ofstream outfile(outputFileName, ios::app);
                    outfile << "# " << num << " #  " << line << ": \n";
                    outfile.close();
                    ListFiles(node, outputFileName);
                }
            }
        }
    }
    // �ر����������ļ�
    infile.close();
    outfile.close();
}