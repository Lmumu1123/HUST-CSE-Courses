#include "tree.h"

/**
 * @brief 转换 FILETIME 时间格式并以标准格式输出到文件
 * @param fileTime 待转换的 FILETIME 结构体
 * @param outfile 输出流对象，用于写入转换后的时间
 */
void PrintFileTime(const FILETIME& fileTime, ofstream& outfile) {
    SYSTEMTIME sysTime;
    FILETIME localFileTime;

    FileTimeToLocalFileTime(&fileTime, &localFileTime); // 将文件时间转换为本地文件时间
    FileTimeToSystemTime(&localFileTime, &sysTime); // 将本地文件时间转换为系统时间

    //cout <<  sysTime.wYear << "年" << sysTime.wMonth << "月" << sysTime.wDay<<"日";
    //cout << sysTime.wHour << ": " << sysTime.wMinute << ": " << sysTime.wSecond  << endl;

    outfile << sysTime.wYear << "年" << sysTime.wMonth << "月" << sysTime.wDay << "日 ";
    outfile << sysTime.wHour << ":" << sysTime.wMinute << ":" << sysTime.wSecond << endl;

}

/**
 * @brief 列出指定目录节点下的所有文件，并统计相关信息写入输出文件
 * @param p 指定目录的树节点
 * @param outputFileName 规定的输出文件名
 * @note 此函数计算目录下文件的总数、大小，以及最早和最晚文件的相关信息
 */
void ListFiles(TreeNode* p, const string& outputFileName) {
    int fileCount = 0;    // 文件总数
    int totalSize = 0;    // 文件总大小
    time_t MAX_time = 0;  // 最早的文件时间
    time_t MIN_time = LONG_TIME;    // 最晚的文件时间
    TreeNode* MAX_node = (TreeNode*)malloc(sizeof(TreeNode)); // 最早的文件节点
    TreeNode* MIN_node = (TreeNode*)malloc(sizeof(TreeNode)); // 最晚的文件节点

    // 遍历目录节点的子节点
    TreeNode* node = p->child;       
    while (node) {
        if (node->isDirectory == 0) { // 如果是文件
            fileCount++;
            totalSize = totalSize + node->size; // 累计文件大小
            // 更新最早和最晚文件的信息
            if (node->time > MAX_time) {
                MAX_time = node->time;
                MAX_node = node;
            }
            if (node->time < MIN_time) {
                MIN_time = node->time;
                MIN_node = node;
            }
        }
        // 移动到下一个兄弟节点
        node = node->sibling;
    }
    //char string_time_max[20] = {};
    //char sting_time_min[20] = {};
    //printf("%d %d %s %s\n",fileCount,totalSize, Microsecond_To_DateTime(MAX_time, string_time_max), Microsecond_To_DateTime(MIN_time, sting_time_min));
    
    // 打开输出文件，准备写入统计信息
    ofstream outfile(outputFileName, ios::app);
    // 写入总文件数和总文件大小
    outfile << "总文件数: " << fileCount << "\n总文件大小: " << totalSize << "B" << endl;
    // 如果有文件，写入最早和最晚的文件信息
    if (fileCount != 0) {
        outfile << "最早文件: " << MAX_node->name << "\n最早文件大小: " << MAX_node->size << "B\n最早时间: ";
        PrintFileTime(MAX_node->file_time, outfile);
        outfile << "最晚文件: " << MIN_node->name << "\n最晚文件大小: " << MIN_node->size << "B\n最晚时间: ";
        PrintFileTime(MIN_node->file_time, outfile);
    }
    else {
        outfile << "最早文件: 无\n最早文件大小: 无\n最早时间: 无" << endl;
        outfile << "最晚文件: 无\n最晚文件大小: 无\n最晚时间: 无 " << endl;

    }
    outfile << endl;
    // 结束后关闭输出文件
    outfile.close();
}

/**
 * @brief 处理输入文件中指定的目录，并将目录下的文件信息输出到指定文件
 * @param inputFileName 输入文件名，其中包含要统计的目录列表
 * @param outputFileName 输出文件名，用来存放统计结果
 * @note 输入文件中的每一行应为一个目录的路径，该函数将统计目录下的文件信息
 */
void ProcessDirectories(const string& inputFileName, const string& outputFileName) {
    ifstream infile(inputFileName);
    ofstream outfile(outputFileName);

    // 检查文件是否成功打开
    if (!infile.is_open() || !outfile.is_open()) {
        cerr << "Error opening input or output file." << endl;
        return;
    }

    bool is_reading_dirs = false; // 标识是否正在读取目录列表
    string line;
    int num = 0;  // 计数器，记录当前处理的是第几个目录

    // 逐行读取输入文件
    while (getline(infile, line)) {
        //cout << "当前行的内容为：" << line << endl;

        if (line == "stat dirs") {
            is_reading_dirs = true; // 开始读取目录列表
            continue;
        }
        else if (line == "end of dirs") {
            is_reading_dirs = false; // 结束读取目录列表
            break;
        }
        // 如果正在读取目录列表
        if (is_reading_dirs) {
            // 处理目录信息
            if (!line.empty() && line.back() == '\\') {
                // 去除路径名结尾的'\'
                line.pop_back();

                num++;
                // 根据路径找到对应的目录节点
                TreeNode* node = findNodeByPath(line);
                if (!node) { // 如果找不到节点，则输出错误信息到文件
                    ofstream outfile(outputFileName, ios::app);
                    outfile << "# " << num << " #  " << line << ":\nError - Unable to open directory\n\n";
                    outfile.close();
                }
                else {  // 如果找到节点，输出目录及其文件信息
                    ofstream outfile(outputFileName, ios::app);
                    outfile << "# " << num << " #  " << line << ": \n";
                    outfile.close();
                    ListFiles(node, outputFileName);
                }
            }
        }
    }
    // 关闭输入和输出文件
    infile.close();
    outfile.close();
}