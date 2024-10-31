#include "all.h"

// �����ջ�ṹ
typedef struct {
    char path[MAX_PATH];     // ��ǰ��������Ŀ¼·��
    int depth;               // ��ǰĿ¼��Ŀ¼���е����
    HANDLE hFind;            // ���ڲ���Ŀ¼���ļ��ľ��
} DirectoryStack;


// ��������ļ���ͳ����Ϣ
typedef struct CFileInfo {
    int numFiles;            // �ļ�����
    int numDirectories;      // Ŀ¼����
    int maxDepth;            // Ŀ¼����������
    char longestPath[MAX_PATH]; // �·��
    int longestPathLength;   // �·���ĳ���
} CFileInfo;


// ������Ŀ¼/�ļ���ɨ��
void listCFiles(const char* path, CFileInfo* Cfiles, ofstream& fileSqlFile, ofstream& dirSqlFile);
// ���ɲ������ݿ��SQL���
void insertIntoSQLFile(char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile);
void insertDirIntoSQLFile(const string& directory, const string& subdirPath, ofstream& dirSqlFile);
// �ļ������
void EXinsertIntoSQLFile(const char* directory, char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile);

// �� FILETIME ��ʽ��ʱ��ת��Ϊ UNIX ʱ���
time_t FileTime_To_Microsecond(FILETIME fileTime);