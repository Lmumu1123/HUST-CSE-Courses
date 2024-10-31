#include "tree.h"

/**
 * @brief 将秒转换为日期时间格式的字符串
 * @param seconds UNIX时间戳（秒）
 * @param dateTimeString 输出参数，存储转换后的日期时间字符串
 * @return 返回指向dateTimeString的指针，如果转换失败则为NULL
 */
char* Microsecond_To_DateTime(time_t seconds, char* dateTimeString) {
    struct tm* dateTimeInfo = localtime(&seconds);
    if (dateTimeInfo != NULL) {
        // 格式化日期时间并存储在 dateTimeString 中
        strftime(dateTimeString, 100, "%Y年%m月%d日 %H:%M:%S", dateTimeInfo);
        return dateTimeString;
    }
    else {
        printf("毫秒转换至日期时间出错。\n");
        return NULL;  // 转换失败返回NULL
    }
}

/**
 * @brief 打印指定目录的信息
 * @param directory 要打印信息的目录路径
 * @note 此函数打印目录的修改时间和大小
 */
void Print(string directory) {
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        cout << "未找到" << directory << " 的信息" << endl;
        return;
    }
    char dateTimeString[100] = "";
    // 转换时间并打印节点信息
    if (Microsecond_To_DateTime(node->time, dateTimeString)) {
        printf("%s 信息为:\n时间：%s 大小：%ldB\n", node->name, dateTimeString, node->size);
    }
    else {
        printf("时间转换错误。\n");
    }
}