#include "all.h"

// 文件信息的数据结构定义
typedef struct {
    char path[MAX_PATH];      // 文件路径
    ULONGLONG size;           // 文件大小，以字节为单位
    FILETIME modifiedTime;    // 文件最后修改时间
} FileInfo;

// 统计指定目录中的文件信息
void ListFiles(TreeNode* p, const string& outputFileName);
void ProcessDirectories(const string& inputFileName, const string& outputFileName);
// 模拟文件操作
void ProcessFile(const string& inputFileName);
// 模拟目录操作
void ProcessDir(const string& inputFileName);
// 打印选择文件的信息
char* Microsecond_To_DateTime(time_t seconds, char* dateTimeString);
void Print(string directory);
// 检查统计信息
void compareFiles(const string& file1, const string& file2);

