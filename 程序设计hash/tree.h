#include "all.h"

// Ŀ¼���ڵ�����ݽṹ����
typedef struct TreeNode {
    char name[MAX_PATH];        // Ŀ¼���ļ���
    int isDirectory;            // ����Ƿ�ΪĿ¼��1ΪĿ¼��0Ϊ�ļ�
    long long size;             // �ļ���С��������ļ��������ֽ�Ϊ��λ
    FILETIME file_time;         // �ļ�����޸�ʱ��
    time_t time;                // ʱ�����ת��Ϊ�ɶ�ʱ���ʽ��
    struct TreeNode* parent;    // ָ�򸸽ڵ��ָ��
    struct TreeNode* child;     // ָ���һ���ӽڵ��ָ��
    struct TreeNode* sibling;   // ָ����һ���ֵܽڵ��ָ��
} TreeNode;


// ���ڴ��й���Ŀ¼����ͳ���ļ���Ϣ
int calculateTreeDepth(TreeNode* root);
void buildTree(TreeNode* parentNode, const char* path);
void freeTree(TreeNode* root);
void printTree(TreeNode* node, int level);

// ����ͨ��ѭ��Ѱ�Ҹ��ڵ㺯��
TreeNode* find_parent(TreeNode* root, TreeNode* node);

// ��ϣ��Ѱ�ҽڵ�
TreeNode* findNodeByPath(const string& path);
void addToIndex(TreeNode* node);
