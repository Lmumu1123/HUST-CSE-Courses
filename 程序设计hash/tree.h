#include "all.h"

// 目录树节点的数据结构定义
typedef struct TreeNode {
    char name[MAX_PATH];        // 目录或文件名
    int isDirectory;            // 标记是否为目录，1为目录，0为文件
    long long size;             // 文件大小（仅针对文件），以字节为单位
    FILETIME file_time;         // 文件最后修改时间
    time_t time;                // 时间戳（转换为可读时间格式）
    struct TreeNode* parent;    // 指向父节点的指针
    struct TreeNode* child;     // 指向第一个子节点的指针
    struct TreeNode* sibling;   // 指向下一个兄弟节点的指针
} TreeNode;


// 在内存中构造目录树，统计文件信息
int calculateTreeDepth(TreeNode* root);
void buildTree(TreeNode* parentNode, const char* path);
void freeTree(TreeNode* root);
void printTree(TreeNode* node, int level);

// 测试通过循环寻找父节点函数
TreeNode* find_parent(TreeNode* root, TreeNode* node);

// 哈希表寻找节点
TreeNode* findNodeByPath(const string& path);
void addToIndex(TreeNode* node);
