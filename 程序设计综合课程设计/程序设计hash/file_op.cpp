#include "tree.h"
extern std::unordered_map<std::string, TreeNode*> nodeIndex;
extern TreeNode* rootNode;

/**
 * @brief 从根节点开始寻找指定节点的父节点
 * @param root 目录树的根节点
 * @param node 要寻找父节点的目标节点
 * @return 返回目标节点的父节点指针，若不存在则返回 NULL
 */
TreeNode* find_parent(TreeNode* root, TreeNode* node) {
    TreeNode* stack[MAX_STACK] = {};
    TreeNode* p;
    int top = -1;
    stack[++top] = root;
    // 使用栈遍历树，查找父节点
    while (top > -1) {
        p = stack[top];
        if (p->child  == node || p->sibling == node) return p;
        top--;
        if (p->sibling) stack[++top] = p->sibling;
        if (p->child) stack[++top] = p->child;
    }
    return NULL; // 没有找到父节点
}

/**
 * @brief 修改指定文件的时间和大小属性
 * @param directory 文件的目录路径
 * @param time 新的时间属性
 * @param size 新的文件大小
 */
void modify_f(string directory, time_t time, long size) {
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        //cerr << "文件不存在，无法修改" << endl;
        return ;
    }
    // 更新节点时间和大小属性
    node->time = time;
    node->size = size;
}

/**
 * @brief 删除指定的文件节点
 * @param directory 文件的目录路径
 */
void delete_f(string directory) {
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        //cerr << "文件不存在，无法删除" << endl;
        return;
    }
    TreeNode* parent_node = node->parent;
    //TreeNode* parent_node = find_parent(rootNode,node);
    // 更新父节点子节点链表，移除当前节点
    if (parent_node->child == node) {
        parent_node->child = node->sibling;
    }
    else {
        parent_node->sibling = node->sibling;
    }
    if (node->sibling) node->sibling->parent = parent_node;

    //将节点从哈希表中移除
    std::string indexKey = std::to_string(std::hash<std::string>{}(node->name));
    nodeIndex.erase(indexKey);
    // 释放节点内存
    delete node; 
}

/**
 * @brief 在指定目录下添加新文件节点
 * @param directory_add 目录的路径
 * @param file_name 新文件的名称
 * @param time 文件的时间属性
 * @param size 文件的大小
 */
void add_f(string directory_add, string file_name, time_t time, long size) {
    TreeNode* node = findNodeByPath(directory_add);
    if (!node) {
        //cerr << "文件目录不存在，无法添加" << endl;
        return;
    }
    // 创建新文件节点并设置属性
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    newNode->time = time;
    strcpy(newNode->name, file_name.c_str());
    newNode->isDirectory = 0;
    newNode->size = size;
    newNode->child = NULL;
    newNode->sibling = NULL;

    // 将新节点添加到目录下
    if (node->child) {
        node = node->child;
        newNode->sibling = node->sibling;
        if (node->sibling) node->sibling->parent = newNode;
        node->sibling = newNode;
        newNode->parent = node;
    }
    else {
        node->child = newNode;
        newNode->parent = node;
    }

    addToIndex(newNode); // 将新节点添加到索引中

}

/**
 * @brief 处理输入文件中的文件操作指令，执行添加、修改和删除操作
 * @param inputFileName 包含文件操作指令的输入文件路径
 */
void ProcessFile(const string& inputFileName) {
    ifstream infile(inputFileName);
    // 打开输入文件，检查是否成功
    if (!infile.is_open()) {
        cerr << "Error opening input or output file." << endl;
        return;
    }
    // 标记是否处于读取文件操作的部分
    bool is_reading_dirs = false;
    string line;
    // 逐行读取输入文件
    while (getline(infile, line)) {
        //cout << "当前行的内容为：" << line << endl;
        if (line == "selected files") {
            is_reading_dirs = true; // 开始处理文件操作
            continue;
        }
        else if (line == "end of files") {
            is_reading_dirs = false; // 结束处理文件操作
            break;
        }
        // 如果当前处于文件操作处理阶段
        if (is_reading_dirs) {
            // 处理目录信息
            if (!line.empty()) {
                // 分解字符串以获取文件的路径、操作模式、时间和大小等信息
                size_t start = line.find("c:\\");
                size_t partition1 = line.find(",");
                size_t partition2 = line.find(",", partition1 + 1);
                size_t partition3 = line.find(",", partition2 + 1);
                size_t end = line.find("\n");

                string directory = line.substr(start, partition1 - start);
                string mode = line.substr(partition1 + 1, 1);
                string timestr = line.substr(partition2 + 1, partition3 - partition2 - 1);
                string sizestr = line.substr(partition3 + 1, end - partition3 - 1);

                //cout << directory << endl;
                // 执行相应的文件操作
                if (mode == "D") delete_f(directory);  // 删除文件操作
                else {
                    time_t time = (time_t)stol(timestr);
                    long size = stol(sizestr);
                    if (mode == "M")  modify_f(directory, time, size); // 修改文件操作
                    else {
                        // 提取添加文件的目录部分
                        string directory_add = directory.substr(start, directory.rfind('\\') - start);
                        add_f(directory_add, directory, time, size); // 添加文件操作
                    }
                }

            }
        }
    }
    // 完成操作后，关闭文件
    infile.close();
}

