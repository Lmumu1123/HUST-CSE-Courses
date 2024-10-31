#include "tree.h"

extern unordered_map<string, TreeNode*> nodeIndex; //声明

/**
 * @brief 将目录树节点添加到哈希表中
 * @param node 要添加到哈希表的目录树节点
 * @note 使用节点的名称的哈希值作为键，节点指针作为值，将其添加到哈希表中
 */
void addToIndex(TreeNode* node) {
    // 对节点的名称进行哈希，得到一个唯一的键值
    string indexKey = to_string(hash<string>{}(node->name));

    // 在哈希表中添加键值对，以便快速查找
    nodeIndex[indexKey] = node;

}

/**
 * @brief 通过路径在哈希表中查找对应的目录树节点
 * @param path 要查找的目录或文件的路径
 * @return 查找到的目录树节点指针；如果未找到，返回 nullptr
 * @note 此函数用于快速定位指定路径的节点
 */
TreeNode* findNodeByPath(const string& path) {
    // 对查找路径进行哈希，得到索引键值
    string indexKey = to_string(hash<string>{}(path));

    // 在哈希表中根据键值查找节点
    auto it = nodeIndex.find(indexKey);
    if (it != nodeIndex.end()) {
        // 如果找到了对应的节点，返回节点指针
        return it->second;
    }
    // 如果没有找到，返回空指针
    return nullptr;
}

/**
 * @brief 将 FILETIME 格式的时间转换为 UNIX 时间戳。
 * @param fileTime 要转换的 FILETIME 结构体。
 * @return 表示 UNIX 时间戳的 time_t 值。
 */
time_t FileTime_To_Microsecond(FILETIME fileTime) {
    const unsigned long long EPOCH_DIFF = 116444736000000000;
    unsigned long long microsecond = ((unsigned long long)fileTime.dwHighDateTime << 32) | (unsigned long long)fileTime.dwLowDateTime;
    time_t seconds = (microsecond - EPOCH_DIFF) / 10000000;
    return seconds;
}

/**
 * @brief 从给定路径和根节点开始递归构建目录树。
 * @param parentNode 用于构建目录树的父节点指针。
 * @param path 起始构建目录树的目录路径。
 */
void buildTree(TreeNode* parentNode, const char* path) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(path, &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error finding file/directory\n");
        return;
    }

    // 使用栈来模拟递归调用过程
    stack<pair<TreeNode*, string>> stack;
    stack.push({ parentNode, path });

    while (!stack.empty()) {
        // 取出栈顶元素，进行处理
        auto current = stack.top();
        stack.pop(); 

        //printf("Processing path: %s\n", current.second.c_str());  // 输出当前处理的路径

        HANDLE hFind = FindFirstFileA(current.second.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            continue;
        }

        do {
            // 忽略当前目录和父目录
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
                continue;
            }

            // 创建新节点并设置属性
            TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
            // 如果路径末尾是"\*"，则去除这部分内容
            if (current.second.length() >= 2 && current.second.substr(current.second.length() - 2) == "\\*") {
                current.second = current.second.substr(0, current.second.length() - 2);
            }
            //cout << current.first << ":" << findNodeByPath(current.second) << endl;
            sprintf(newNode->name, "%s\\%s", current.second.c_str(), findData.cFileName);
            newNode->isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            newNode->size = findData.nFileSizeLow;
            newNode->file_time = findData.ftLastWriteTime;
            newNode->time = FileTime_To_Microsecond(findData.ftLastWriteTime);
            newNode->child = NULL;
            newNode->sibling = NULL;

            // 将节点添加到哈希表中
            addToIndex(newNode);

            //printf("Adding node: %s, isDirectory: %d, size: %d\n", newNode->name, newNode->isDirectory, newNode->size);  // 输出添加的节点信息

            // 将新节点添加为其父节点的子节点
            if (current.first->child == NULL) {
                current.first->child = newNode;
                newNode->parent = current.first;
            }
            else {
                TreeNode* currentSibling = current.first->child;
                while (currentSibling->sibling != NULL) {
                    currentSibling = currentSibling->sibling;
                }
                currentSibling->sibling = newNode;
                newNode->parent = currentSibling;
            }

            // 如果是目录，则将其子目录入栈，继续遍历
            if (newNode->isDirectory) {
                char childPath[MAX_PATH];
                snprintf(childPath, MAX_PATH, "%s\\%s\\*", current.second.c_str(), findData.cFileName);
                //snprintf(childPath, MAX_PATH, "%s\\%s\\*", current.second.c_str(), findData.cFileName);
                //printf("childPath:%s\n", childPath);
                stack.push({ newNode, childPath });
            }
        } while (FindNextFileA(hFind, &findData) != 0);

        FindClose(hFind);
    }
}

/**
 * @brief 计算目录树的深度。
 * @param root 目录树的根节点指针。
 * @return 目录树的最大深度。
 */
int calculateTreeDepth(TreeNode* root) {
    if (root == nullptr) {
        return 0;
    }

    queue<TreeNode*> q;
    q.push(root);
    int depth = 0;

    while (!q.empty()) {
        int size = q.size();   // 当前层的节点数

        for (int i = 0; i < size; ++i) {
            TreeNode* node = q.front();
            q.pop();

            if (node->child != nullptr) {
                q.push(node->child);

            }

            TreeNode* sibling = node->sibling;
            if (sibling != nullptr) {
                q.push(sibling);
            }

        }
        ++depth;  // 每遍历完一层，深度加一
    }
    return depth;
}

/**
 * @brief 递归释放目录树中的节点及其所有子节点占用的内存
 * @param root 目录树的根节点指针
 * @note 此函数对目录树进行后序遍历，保证在释放父节点之前子节点已经被释放
 */
void freeTree(TreeNode* root) {
    if (root == NULL) {
        return;
    }

    TreeNode* currentChild = root->child;
    while (currentChild != NULL) {
        TreeNode* nextChild = currentChild->sibling;
        freeTree(currentChild);   // 首先递归释放子节点
        free(currentChild);       // 然后释放当前节点
        currentChild = nextChild; // 移动到兄弟节点
    }
}

/**
 * @brief 检查并打印目录树结构
 * @param node 当前需要打印的目录树节点
 * @param level 对应的层级（用于打印缩进）
 * @note 此函数使用前序遍历的方式打印目录树，可以直观地查看树结构
 */
void printTree(TreeNode* node, int level) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < level; i++) {
        printf("  "); // 打印空格作为缩进
    }
    // 打印节点信息：节点名，是目录还是文件，节点大小
    printf("|- %s (%s, %d bytes)\n", node->name, node->isDirectory ? "Directory" : "File", node->size);

    // 递归打印子节点和兄弟节点
    printTree(node->child, level + 1); // 子节点层级加一
    printTree(node->sibling, level);   // 兄弟节点层级不变
}