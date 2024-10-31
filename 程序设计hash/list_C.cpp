#include "scanC.h"

/**
 * @brief ����ǰ·�����������Ϣѹ��Ŀ¼������ջ
 * @param stack Ŀ¼��ջ
 * @param path ��ǰ���ڱ�����Ŀ¼·��
 * @param depth ��ǰĿ¼��Ŀ¼���е����
 * @param top ָ���ջ����������ָ��
 * @param Cfiles �洢�ļ�ͳ����Ϣ�Ľṹ��ָ��
 * @note �����ջ���������ӡ��ջ����Ĵ�����Ϣ
 */
void push(DirectoryStack* stack, const char* path, int depth, int* top, CFileInfo* Cfiles) {
    if (*top < MAX_DEPTH - 1) {
        strcpy(stack[*top].path, path);
        stack[*top].depth = depth;
        (*top)++;
        if (*top > Cfiles->maxDepth) {
            Cfiles->maxDepth = *top;
        }
    }
    else {
        printf("��ջ�����\n");
    }
}

/**
 * @brief ��Ŀ¼������ջ�е�������Ԫ��
 * @param top ָ���ջ����������ָ��
 * @param stack Ŀ¼��ջ
 * @note �����ջ������Ӧ�Ĳ��Ҿ����Ч�������ȹرոþ��
 */
void pop(int* top, DirectoryStack* stack) {
    if (stack[*top].hFind != INVALID_HANDLE_VALUE) FindClose(stack[*top].hFind);   // �ر�Windows���Ҿ��
    (*top)--;  // ���ٶ�ջ��������
}

/**
 * @brief ɨ��ָ��·���µ������ļ���Ŀ¼��������Ϣ�����SQL�ļ�
 * @param path Ҫɨ���Ŀ¼·��
 * @param Cfiles �洢�ļ���Ŀ¼��Ϣ�Ľṹ��ָ��
 * @param fileSqlFile �ļ���Ϣ�����SQL�ļ���
 * @param dirSqlFile Ŀ¼��Ϣ�����SQL�ļ���
 * @note �˺������÷ǵݹ����Ŀ¼�ķ�ʽ�����ö�ջ������������е�״̬
 */
void listCFiles(const char* path, CFileInfo* Cfiles, ofstream& fileSqlFile, ofstream& dirSqlFile) {
    WIN32_FIND_DATAA findData;          // ���ڴ����ļ���Ϣ
    char newPath[MAX_PATH];             // ��·������
    DirectoryStack stack[MAX_DEPTH];    // Ŀ¼������ջ
    int top = 0;                        // ��ջ��������
    BOOL flag = true;                   // ��־���������ڿ���ѭ��

    // ��ʼ��CFileInfo�ṹ
    Cfiles->numFiles = 0;
    Cfiles->numDirectories = 0;
    Cfiles->maxDepth = 0;
    Cfiles->longestPath[MAX_PATH];
    Cfiles->longestPathLength = 0;

    int fileRecordsCount = 0;           // �ļ���¼������
    int dirRecordsCount = 0;            // Ŀ¼��¼������

    // �Ӹ�Ŀ¼��ʼ��ѹ���ջ���б���
    push(stack, path, 0, &top, Cfiles);
    sprintf(newPath, "%s/*", path);
    stack[top].hFind = FindFirstFileA(newPath, &findData);
    HANDLE hFind = stack[top].hFind; // ��ȡ���Ҿ��
    if (hFind != INVALID_HANDLE_VALUE) {
        do { // ѭ����ֱ�������ļ���Ŀ¼��������
            do {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) { // ���� "." �� ".." ����������Ŀ
                    if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { // ������ļ�������д���
                        (Cfiles->numFiles)++;

                        sprintf(newPath, "%s/%s", stack[top - 1].path, findData.cFileName);
                        // ��ȡ�ļ�·��

                        // �ļ�·���������ģ��ַ����ȴ�unicodeת��ΪUTF-8
                        // ת��Ϊ���ַ���Unicode��
                        int size_needed = MultiByteToWideChar(CP_ACP, 0, newPath, -1, NULL, 0);
                        wchar_t* wpath = new wchar_t[size_needed];
                        MultiByteToWideChar(CP_ACP, 0, newPath, -1, wpath, size_needed);

                        // ת��ΪUTF-8
                        size_needed = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
                        char* utf8path = new char[size_needed];
                        WideCharToMultiByte(CP_UTF8, 0, wpath, -1, utf8path, size_needed, NULL, NULL);

                        //string file_path = newPath;

                        //if (isContainChinese(file_path)) continue;

                        // ��ȡ�ļ���С
                        uintmax_t file_size = findData.nFileSizeLow;
                        // ��ȡ�ļ��޸�ʱ��
                 
                        time_t file_time = FileTime_To_Microsecond(findData.ftLastWriteTime); // �ļ�ʱ��ת��

                        EXinsertIntoSQLFile(stack[top - 1].path, utf8path, file_size, file_time, fileSqlFile);
                        // �������ı������
                        //EXinsertIntoSQLFile(stack[top - 1].path, newPath, file_size, file_time, fileSqlFile);
                        // insertIntoSQLFile(utf8path, file_size, file_time, fileSqlFile);
                        fileRecordsCount++;
                        if (fileRecordsCount >= MAX_SQL_FILE) {
                            fileSqlFile.close(); // �رյ�ǰ�ļ� SQL �ļ�
                            string nextFileName = "./sql_file/file_" + to_string(Cfiles->numFiles / MAX_SQL_FILE + 1) + ".sql";
                            fileSqlFile.open(nextFileName); // ����һ���ļ� SQL �ļ�
                            if (!fileSqlFile.is_open()) {
                                cerr << "Error: Unable to open file SQL file" << endl;
                                FindClose(hFind);
                                return;
                            }
                            fileRecordsCount = 0; // ���ü�¼������
                        }

                        push(stack, newPath, top, &top, Cfiles);

                        if (strlen(newPath) > Cfiles->longestPathLength) {
                            strcpy(Cfiles->longestPath, newPath);
                            Cfiles->longestPathLength = strlen(Cfiles->longestPath);
                        }
                        top--;
                        flag = FindNextFileA(hFind, &findData);

                    }
                    else { // �����Ŀ¼������д���
                        (Cfiles->numDirectories)++;
                        sprintf(newPath, "%s/%s", stack[top - 1].path, findData.cFileName);
                        push(stack, newPath, top, &top, Cfiles);

                        insertDirIntoSQLFile(path, newPath, dirSqlFile);
                        dirRecordsCount++;
                        if (dirRecordsCount >= MAX_SQL_FILE) {
                            dirSqlFile.close(); // �رյ�ǰĿ¼ SQL �ļ�
                            string nextFileName = "./sql_dir/dir_" + to_string(Cfiles->numDirectories / MAX_SQL_FILE + 1) + ".sql";
                            dirSqlFile.open(nextFileName); // ����һ��Ŀ¼ SQL �ļ�
                            if (!dirSqlFile.is_open()) {
                                cerr << "Error: Unable to open directory SQL file" << endl;
                                FindClose(hFind);
                                return;
                            }
                            dirRecordsCount = 0; // ���ü�¼������
                        }

                        //��·������
                        sprintf(newPath, "%s/*", newPath);
                        stack[top].hFind = FindFirstFileA(newPath, &findData);
                        hFind = stack[top].hFind;
                        if (hFind == INVALID_HANDLE_VALUE) {
                            DWORD error = GetLastError();
                            top--;
                            hFind = stack[top].hFind;
                            flag = FindNextFileA(hFind, &findData);
                            if (error == ERROR_ACCESS_DENIED) {
                                printf("ACCESS_DENIED��%s\n", stack[top].path);
                                continue;
                            }
                            else {
                                printf("FindFirstFileA��������ţ�%d\n", error);
                                continue;
                            }
                        }
                    }
                }
                else {
                    flag = FindNextFileA(hFind, &findData);
                }//���ļ��н��������˳�
            } while (flag != FALSE);
            flag = true;        //����flagΪtrue
            top--;
            if (top == 0) return;      // �����ջΪ�գ���ʾ��������

            // ������һ��Ŀ¼���ļ�
            hFind = stack[top].hFind;
            while (FindNextFileA(hFind, &findData) == FALSE) {
                pop(&top, stack);
                if (top == 0) return;
                hFind = stack[top].hFind;
            }
        } while (top > 0);

    }

}