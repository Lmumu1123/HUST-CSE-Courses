#include "all.h"

// �ļ���Ϣ�����ݽṹ����
typedef struct {
    char path[MAX_PATH];      // �ļ�·��
    ULONGLONG size;           // �ļ���С�����ֽ�Ϊ��λ
    FILETIME modifiedTime;    // �ļ�����޸�ʱ��
} FileInfo;

// ͳ��ָ��Ŀ¼�е��ļ���Ϣ
void ListFiles(TreeNode* p, const string& outputFileName);
void ProcessDirectories(const string& inputFileName, const string& outputFileName);
// ģ���ļ�����
void ProcessFile(const string& inputFileName);
// ģ��Ŀ¼����
void ProcessDir(const string& inputFileName);
// ��ӡѡ���ļ�����Ϣ
char* Microsecond_To_DateTime(time_t seconds, char* dateTimeString);
void Print(string directory);
// ���ͳ����Ϣ
void compareFiles(const string& file1, const string& file2);

