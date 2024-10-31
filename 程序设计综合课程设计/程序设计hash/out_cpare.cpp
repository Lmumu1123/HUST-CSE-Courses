#include "all.h"

/**
 * @brief 比较两个模块的统计信息并打印变化情况
 * @param module1 原始模块的统计信息字符串
 * @param module2 比较模块的统计信息字符串
 * @param name 被比较的文件目录名称
 */
void processModuleInfo(const string& module1, const string& module2, const string& name) {

    if (module2.find("Error - Unable to open directory") != string::npos) {
        cout << name << "原信息为\n" << module1 << "---->目标目录已删除" << endl;
        return;
    }

    // 使用正则表达式匹配特定格式的内容
    smatch match;
    regex reg(R"(总文件数: (\d+)\n.*总文件大小: (\d+)B\n.*最早文件: (.*)\n.*最早文件大小: (.*)\n.*最早时间: (.*)\n.*最晚文件: (.*)\n.*最晚文件大小: (.*)\n.*最晚时间: (.*)\n)");

    if (regex_search(module1, match, reg)) {
        // 提取各字段信息
        string totalFiles1 = match.str(1);
        string totalSize1 = match.str(2);
        string earliestFile1 = match.str(3);
        string earliestSize1 = match.str(4);
        string earliestTime1 = match.str(5);
        string latestFile1 = match.str(6);
        string latestSize1 = match.str(7);
        string latestTime1 = match.str(8);

        regex_search(module2, match, reg); // 正则匹配第二个模块
        string totalFiles2 = match.str(1);
        string totalSize2 = match.str(2);
        string earliestFile2 = match.str(3);
        string earliestSize2 = match.str(4);
        string earliestTime2 = match.str(5);
        string latestFile2 = match.str(6);
        string latestSize2 = match.str(7);
        string latestTime2 = match.str(8);

        // 比较各字段，并且输出不同的内容
        if (totalFiles1 != totalFiles2 || totalSize1 != totalSize2 || earliestFile1 != earliestFile2 || earliestSize1 != earliestSize2 ||
            earliestTime1 != earliestTime2 || latestFile1 != latestFile2 || latestSize1 != latestSize2 || latestTime1 != latestTime2) {
            cout << name << endl;

            // 比较字段内容并输出差异
            if (totalFiles1 != totalFiles2) {
                cout << " 总文件数 - " << totalFiles1 << " -> " << totalFiles2 << endl;
            }
            if (totalSize1 != totalSize2) {
                cout << " 总文件大小 - " << totalSize1 << "B -> " << totalSize2 << "B" << endl;
            }
            if (earliestFile1 != earliestFile2) {
                cout << " 最早文件 - " << earliestFile1 << " -> " << earliestFile2 << endl;
            }
            if (earliestSize1 != earliestSize2) {
                cout << " 最早文件大小 - " << earliestSize1 << " -> " << earliestSize2 << endl;
            }
            if (earliestTime1 != earliestTime2) {
                cout << " 最早时间 - " << earliestTime1 << " -> " << earliestTime2 << endl;
            }
            if (latestFile1 != latestFile2) {
                cout << " 最晚文件 - " << latestFile1 << " -> " << latestFile2 << endl;
            }
            if (latestSize1 != latestSize2) {
                cout << " 最晚文件大小 - " << latestSize1 << " -> " << latestSize2 << endl;
            }
            if (latestTime1 != latestTime2) {
                cout << " 最晚时间 - " << latestTime1 << " -> " << latestTime2 << endl;
            }
        }

    }
}

/**
 * @brief 比较两个文件中的统计信息变化
 * @param file1 第一个文件的路径
 * @param file2 第二个文件的路径
 */
void compareFiles(const string& file1, const string& file2) {
    // 打开两个文件进行比较
    ifstream input1(file1);
    ifstream input2(file2);

    // 检查文件是否成功打开
    if (!input1.is_open() || !input2.is_open()) {
        //cerr << "Error opening files!" << endl;
        return;
    }

    string line1, line2; // 用于存储每行内容
    string name; //用来保留文件路径
    string module1;
    string module2;

    while (getline(input1, line1) && getline(input2, line2)) {
        if (line1.find("# ") == 0) {
            name = line1;
            continue;
        }
        // 将完整的模块信息读取到 module1 和 module2 中
        while (!line1.empty()) {
            module1 += line1 + "\n";
            getline(input1, line1);
        }
        while (!line2.empty()) {
            module2 += line2 + "\n";
            getline(input2, line2);
        }
        // 如果两个文件中同时出现空行，意味着一个模块的结束
        if (line1.empty() && line2.empty()) {
            // 处理完整的一个模块信息
            if (!module1.empty()) {
                processModuleInfo(module1, module2, name);
                module1.clear();  // 清空模块信息，准备下一次比较
                module2.clear();
            }
        }
    }
    // 关闭文件流
    input1.close();
    input2.close();
}