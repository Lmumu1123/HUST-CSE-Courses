#include "all.h"

// 定义堆栈结构
typedef struct {
    char path[MAX_PATH];     // 当前遍历到的目录路径
    int depth;               // 当前目录在目录树中的深度
    HANDLE hFind;            // 用于查找目录中文件的句柄
} DirectoryStack;


// 定义磁盘文件的统计信息
typedef struct CFileInfo {
    int numFiles;            // 文件总数
    int numDirectories;      // 目录总数
    int maxDepth;            // 目录树的最大深度
    char longestPath[MAX_PATH]; // 最长路径
    int longestPathLength;   // 最长路径的长度
} CFileInfo;


// 磁盘中目录/文件的扫描
void listCFiles(const char* path, CFileInfo* Cfiles, ofstream& fileSqlFile, ofstream& dirSqlFile);
// 生成插入数据库的SQL语句
void insertIntoSQLFile(char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile);
void insertDirIntoSQLFile(const string& directory, const string& subdirPath, ofstream& dirSqlFile);
// 文件表含外键
void EXinsertIntoSQLFile(const char* directory, char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile);

// 将 FILETIME 格式的时间转换为 UNIX 时间戳
time_t FileTime_To_Microsecond(FILETIME fileTime);