#include "all.h"

/**
 * @brief �Ƚ�����ģ���ͳ����Ϣ����ӡ�仯���
 * @param module1 ԭʼģ���ͳ����Ϣ�ַ���
 * @param module2 �Ƚ�ģ���ͳ����Ϣ�ַ���
 * @param name ���Ƚϵ��ļ�Ŀ¼����
 */
void processModuleInfo(const string& module1, const string& module2, const string& name) {

    if (module2.find("Error - Unable to open directory") != string::npos) {
        cout << name << "ԭ��ϢΪ\n" << module1 << "---->Ŀ��Ŀ¼��ɾ��" << endl;
        return;
    }

    // ʹ��������ʽƥ���ض���ʽ������
    smatch match;
    regex reg(R"(���ļ���: (\d+)\n.*���ļ���С: (\d+)B\n.*�����ļ�: (.*)\n.*�����ļ���С: (.*)\n.*����ʱ��: (.*)\n.*�����ļ�: (.*)\n.*�����ļ���С: (.*)\n.*����ʱ��: (.*)\n)");

    if (regex_search(module1, match, reg)) {
        // ��ȡ���ֶ���Ϣ
        string totalFiles1 = match.str(1);
        string totalSize1 = match.str(2);
        string earliestFile1 = match.str(3);
        string earliestSize1 = match.str(4);
        string earliestTime1 = match.str(5);
        string latestFile1 = match.str(6);
        string latestSize1 = match.str(7);
        string latestTime1 = match.str(8);

        regex_search(module2, match, reg); // ����ƥ��ڶ���ģ��
        string totalFiles2 = match.str(1);
        string totalSize2 = match.str(2);
        string earliestFile2 = match.str(3);
        string earliestSize2 = match.str(4);
        string earliestTime2 = match.str(5);
        string latestFile2 = match.str(6);
        string latestSize2 = match.str(7);
        string latestTime2 = match.str(8);

        // �Ƚϸ��ֶΣ����������ͬ������
        if (totalFiles1 != totalFiles2 || totalSize1 != totalSize2 || earliestFile1 != earliestFile2 || earliestSize1 != earliestSize2 ||
            earliestTime1 != earliestTime2 || latestFile1 != latestFile2 || latestSize1 != latestSize2 || latestTime1 != latestTime2) {
            cout << name << endl;

            // �Ƚ��ֶ����ݲ��������
            if (totalFiles1 != totalFiles2) {
                cout << " ���ļ��� - " << totalFiles1 << " -> " << totalFiles2 << endl;
            }
            if (totalSize1 != totalSize2) {
                cout << " ���ļ���С - " << totalSize1 << "B -> " << totalSize2 << "B" << endl;
            }
            if (earliestFile1 != earliestFile2) {
                cout << " �����ļ� - " << earliestFile1 << " -> " << earliestFile2 << endl;
            }
            if (earliestSize1 != earliestSize2) {
                cout << " �����ļ���С - " << earliestSize1 << " -> " << earliestSize2 << endl;
            }
            if (earliestTime1 != earliestTime2) {
                cout << " ����ʱ�� - " << earliestTime1 << " -> " << earliestTime2 << endl;
            }
            if (latestFile1 != latestFile2) {
                cout << " �����ļ� - " << latestFile1 << " -> " << latestFile2 << endl;
            }
            if (latestSize1 != latestSize2) {
                cout << " �����ļ���С - " << latestSize1 << " -> " << latestSize2 << endl;
            }
            if (latestTime1 != latestTime2) {
                cout << " ����ʱ�� - " << latestTime1 << " -> " << latestTime2 << endl;
            }
        }

    }
}

/**
 * @brief �Ƚ������ļ��е�ͳ����Ϣ�仯
 * @param file1 ��һ���ļ���·��
 * @param file2 �ڶ����ļ���·��
 */
void compareFiles(const string& file1, const string& file2) {
    // �������ļ����бȽ�
    ifstream input1(file1);
    ifstream input2(file2);

    // ����ļ��Ƿ�ɹ���
    if (!input1.is_open() || !input2.is_open()) {
        //cerr << "Error opening files!" << endl;
        return;
    }

    string line1, line2; // ���ڴ洢ÿ������
    string name; //���������ļ�·��
    string module1;
    string module2;

    while (getline(input1, line1) && getline(input2, line2)) {
        if (line1.find("# ") == 0) {
            name = line1;
            continue;
        }
        // ��������ģ����Ϣ��ȡ�� module1 �� module2 ��
        while (!line1.empty()) {
            module1 += line1 + "\n";
            getline(input1, line1);
        }
        while (!line2.empty()) {
            module2 += line2 + "\n";
            getline(input2, line2);
        }
        // ��������ļ���ͬʱ���ֿ��У���ζ��һ��ģ��Ľ���
        if (line1.empty() && line2.empty()) {
            // ����������һ��ģ����Ϣ
            if (!module1.empty()) {
                processModuleInfo(module1, module2, name);
                module1.clear();  // ���ģ����Ϣ��׼����һ�αȽ�
                module2.clear();
            }
        }
    }
    // �ر��ļ���
    input1.close();
    input2.close();
}