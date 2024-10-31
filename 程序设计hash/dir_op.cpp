#include "tree.h"
extern std::unordered_map<std::string, TreeNode*> nodeIndex;
extern TreeNode* rootNode;

/**
 * @brief 递归地释放目录树节点及其子节点的内存，并从哈希表中删除对应项
 * @param root 目录树的根节点，将从此节点开始释放
 */
void freeDir(TreeNode* root) {
    if (root == NULL) {
        return;     // 如果节点为空，则直接返回
    }

    // 遍历所有子节点并递归释放
    TreeNode* currentChild = root->child;
    while (currentChild != NULL) {
        TreeNode* nextChild = currentChild->sibling;
        freeDir(currentChild); // 递归释放子节点

        // 将节点名称从哈希表中删除
        std::string indexKey = std::to_string(std::hash<std::string>{}(currentChild->name));
        nodeIndex.erase(indexKey);

        free(currentChild); // 释放当前节点的内存
        currentChild = nextChild; // 继续处理兄弟节点
    }
}

/**
 * @brief 删除指定的目录及其所有子节点
 * @param directory 要删除的目录的路径
 * @note 该函数会更新目录节点的父子关系，并且释放所有相关节点的内存
 */
void delete_d(string directory) {
    // 通过路径找到指定的节点
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        //cerr << "文件不存在，无法删除" << endl;
        return;
    }
    // 更新父节点的子节点链表，从中移除当前节点
    TreeNode* parent_node = node->parent;
    //TreeNode* parent_node = find_parent(rootNode,node);
    if (parent_node->child == node) {
        parent_node->child = node->sibling;
    }
    else {
        parent_node->sibling = node->sibling;
    }
    // 如果有兄弟节点，更新兄弟节点的父指针
    if (node->sibling) node->sibling->parent = parent_node;

    // 释放该目录及其所有子节点的内存
    freeDir(node);
    // 将节点从哈希表中移除
    std::string indexKey = std::to_string(std::hash<std::string>{}(node->name));
    nodeIndex.erase(indexKey);
    // 删除节点
    delete node;

}

/**
 * @brief 处理输入文件中列举的所有目录，并执行删除操作
 * @param inputFileName 输入文件的名称，该文件包含要删除的目录列表
 * @note 文件中每一行表示一个要删除的目录路径
 */
void ProcessDir(const string& inputFileName) {
    ifstream infile(inputFileName);

    if (!infile.is_open()) {
        cerr << "Error opening input file." << endl;
        return;
    }

    bool is_reading_dirs = false;
    string line;
    int num = 0;

    // 逐行读取输入文件
    while (getline(infile, line)) {
        //cout << "当前行的内容为：" << line << endl;

        if (line == "selected dirs") {
            is_reading_dirs = true;
            continue;
        }
        else if (line == "end of dirs") {
            is_reading_dirs = false;
            break;
        }

        if (is_reading_dirs) {
            // 处理目录信息
            if (!line.empty()) {
                size_t start = line.find("c:\\");
                size_t partition1 = line.find(",");//第一个逗号

                string directory = line.substr(start, partition1 - start - 1);

                //cout << directory << endl;
                 // 执行删除目录操作
                delete_d(directory);
            }
        }
    }
    infile.close();// 关闭输入文件
}

