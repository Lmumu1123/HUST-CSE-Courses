#include "scanC.h"

/**
 * @brief 将当前路径及其相关信息压入目录遍历堆栈
 * @param stack 目录堆栈
 * @param path 当前正在遍历的目录路径
 * @param depth 当前目录在目录树中的深度
 * @param top 指向堆栈顶部索引的指针
 * @param Cfiles 存储文件统计信息的结构体指针
 * @note 如果堆栈已满，会打印堆栈溢出的错误信息
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
        printf("堆栈溢出！\n");
    }
}

/**
 * @brief 从目录遍历堆栈中弹出顶部元素
 * @param top 指向堆栈顶部索引的指针
 * @param stack 目录堆栈
 * @note 如果堆栈顶部对应的查找句柄有效，会首先关闭该句柄
 */
void pop(int* top, DirectoryStack* stack) {
    if (stack[*top].hFind != INVALID_HANDLE_VALUE) FindClose(stack[*top].hFind);   // 关闭Windows查找句柄
    (*top)--;  // 减少堆栈顶部索引
}

/**
 * @brief 扫描指定路径下的所有文件和目录，并将信息输出至SQL文件
 * @param path 要扫描的目录路径
 * @param Cfiles 存储文件和目录信息的结构体指针
 * @param fileSqlFile 文件信息输出的SQL文件流
 * @param dirSqlFile 目录信息输出的SQL文件流
 * @note 此函数采用非递归遍历目录的方式，利用堆栈管理遍历过程中的状态
 */
void listCFiles(const char* path, CFileInfo* Cfiles, ofstream& fileSqlFile, ofstream& dirSqlFile) {
    WIN32_FIND_DATAA findData;          // 用于储存文件信息
    char newPath[MAX_PATH];             // 新路径缓存
    DirectoryStack stack[MAX_DEPTH];    // 目录遍历堆栈
    int top = 0;                        // 堆栈顶部索引
    BOOL flag = true;                   // 标志变量，用于控制循环

    // 初始化CFileInfo结构
    Cfiles->numFiles = 0;
    Cfiles->numDirectories = 0;
    Cfiles->maxDepth = 0;
    Cfiles->longestPath[MAX_PATH];
    Cfiles->longestPathLength = 0;

    int fileRecordsCount = 0;           // 文件记录计数器
    int dirRecordsCount = 0;            // 目录记录计数器

    // 从根目录开始，压入堆栈进行遍历
    push(stack, path, 0, &top, Cfiles);
    sprintf(newPath, "%s/*", path);
    stack[top].hFind = FindFirstFileA(newPath, &findData);
    HANDLE hFind = stack[top].hFind; // 获取查找句柄
    if (hFind != INVALID_HANDLE_VALUE) {
        do { // 循环，直到所有文件和目录都被遍历
            do {
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) { // 忽略 "." 和 ".." 两个特殊条目
                    if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { // 如果是文件，则进行处理
                        (Cfiles->numFiles)++;

                        sprintf(newPath, "%s/%s", stack[top - 1].path, findData.cFileName);
                        // 获取文件路径

                        // 文件路径含有中文，字符需先从unicode转换为UTF-8
                        // 转换为宽字符（Unicode）
                        int size_needed = MultiByteToWideChar(CP_ACP, 0, newPath, -1, NULL, 0);
                        wchar_t* wpath = new wchar_t[size_needed];
                        MultiByteToWideChar(CP_ACP, 0, newPath, -1, wpath, size_needed);

                        // 转换为UTF-8
                        size_needed = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
                        char* utf8path = new char[size_needed];
                        WideCharToMultiByte(CP_UTF8, 0, wpath, -1, utf8path, size_needed, NULL, NULL);

                        //string file_path = newPath;

                        //if (isContainChinese(file_path)) continue;

                        // 获取文件大小
                        uintmax_t file_size = findData.nFileSizeLow;
                        // 获取文件修改时间
                 
                        time_t file_time = FileTime_To_Microsecond(findData.ftLastWriteTime); // 文件时间转换

                        EXinsertIntoSQLFile(stack[top - 1].path, utf8path, file_size, file_time, fileSqlFile);
                        // 出现中文编码错误
                        //EXinsertIntoSQLFile(stack[top - 1].path, newPath, file_size, file_time, fileSqlFile);
                        // insertIntoSQLFile(utf8path, file_size, file_time, fileSqlFile);
                        fileRecordsCount++;
                        if (fileRecordsCount >= MAX_SQL_FILE) {
                            fileSqlFile.close(); // 关闭当前文件 SQL 文件
                            string nextFileName = "./sql_file/file_" + to_string(Cfiles->numFiles / MAX_SQL_FILE + 1) + ".sql";
                            fileSqlFile.open(nextFileName); // 打开下一个文件 SQL 文件
                            if (!fileSqlFile.is_open()) {
                                cerr << "Error: Unable to open file SQL file" << endl;
                                FindClose(hFind);
                                return;
                            }
                            fileRecordsCount = 0; // 重置记录计数器
                        }

                        push(stack, newPath, top, &top, Cfiles);

                        if (strlen(newPath) > Cfiles->longestPathLength) {
                            strcpy(Cfiles->longestPath, newPath);
                            Cfiles->longestPathLength = strlen(Cfiles->longestPath);
                        }
                        top--;
                        flag = FindNextFileA(hFind, &findData);

                    }
                    else { // 如果是目录，则进行处理
                        (Cfiles->numDirectories)++;
                        sprintf(newPath, "%s/%s", stack[top - 1].path, findData.cFileName);
                        push(stack, newPath, top, &top, Cfiles);

                        insertDirIntoSQLFile(path, newPath, dirSqlFile);
                        dirRecordsCount++;
                        if (dirRecordsCount >= MAX_SQL_FILE) {
                            dirSqlFile.close(); // 关闭当前目录 SQL 文件
                            string nextFileName = "./sql_dir/dir_" + to_string(Cfiles->numDirectories / MAX_SQL_FILE + 1) + ".sql";
                            dirSqlFile.open(nextFileName); // 打开下一个目录 SQL 文件
                            if (!dirSqlFile.is_open()) {
                                cerr << "Error: Unable to open directory SQL file" << endl;
                                FindClose(hFind);
                                return;
                            }
                            dirRecordsCount = 0; // 重置记录计数器
                        }

                        //新路径搜索
                        sprintf(newPath, "%s/*", newPath);
                        stack[top].hFind = FindFirstFileA(newPath, &findData);
                        hFind = stack[top].hFind;
                        if (hFind == INVALID_HANDLE_VALUE) {
                            DWORD error = GetLastError();
                            top--;
                            hFind = stack[top].hFind;
                            flag = FindNextFileA(hFind, &findData);
                            if (error == ERROR_ACCESS_DENIED) {
                                printf("ACCESS_DENIED：%s\n", stack[top].path);
                                continue;
                            }
                            else {
                                printf("FindFirstFileA出错，错误号：%d\n", error);
                                continue;
                            }
                        }
                    }
                }
                else {
                    flag = FindNextFileA(hFind, &findData);
                }//空文件夹将从这里退出
            } while (flag != FALSE);
            flag = true;        //重置flag为true
            top--;
            if (top == 0) return;      // 如果堆栈为空，表示遍历结束

            // 处理下一层目录或文件
            hFind = stack[top].hFind;
            while (FindNextFileA(hFind, &findData) == FALSE) {
                pop(&top, stack);
                if (top == 0) return;
                hFind = stack[top].hFind;
            }
        } while (top > 0);

    }

}