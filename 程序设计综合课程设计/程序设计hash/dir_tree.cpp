#include "tree.h"

extern unordered_map<string, TreeNode*> nodeIndex; //����

/**
 * @brief ��Ŀ¼���ڵ���ӵ���ϣ����
 * @param node Ҫ��ӵ���ϣ���Ŀ¼���ڵ�
 * @note ʹ�ýڵ�����ƵĹ�ϣֵ��Ϊ�����ڵ�ָ����Ϊֵ��������ӵ���ϣ����
 */
void addToIndex(TreeNode* node) {
    // �Խڵ�����ƽ��й�ϣ���õ�һ��Ψһ�ļ�ֵ
    string indexKey = to_string(hash<string>{}(node->name));

    // �ڹ�ϣ������Ӽ�ֵ�ԣ��Ա���ٲ���
    nodeIndex[indexKey] = node;

}

/**
 * @brief ͨ��·���ڹ�ϣ���в��Ҷ�Ӧ��Ŀ¼���ڵ�
 * @param path Ҫ���ҵ�Ŀ¼���ļ���·��
 * @return ���ҵ���Ŀ¼���ڵ�ָ�룻���δ�ҵ������� nullptr
 * @note �˺������ڿ��ٶ�λָ��·���Ľڵ�
 */
TreeNode* findNodeByPath(const string& path) {
    // �Բ���·�����й�ϣ���õ�������ֵ
    string indexKey = to_string(hash<string>{}(path));

    // �ڹ�ϣ���и��ݼ�ֵ���ҽڵ�
    auto it = nodeIndex.find(indexKey);
    if (it != nodeIndex.end()) {
        // ����ҵ��˶�Ӧ�Ľڵ㣬���ؽڵ�ָ��
        return it->second;
    }
    // ���û���ҵ������ؿ�ָ��
    return nullptr;
}

/**
 * @brief �� FILETIME ��ʽ��ʱ��ת��Ϊ UNIX ʱ�����
 * @param fileTime Ҫת���� FILETIME �ṹ�塣
 * @return ��ʾ UNIX ʱ����� time_t ֵ��
 */
time_t FileTime_To_Microsecond(FILETIME fileTime) {
    const unsigned long long EPOCH_DIFF = 116444736000000000;
    unsigned long long microsecond = ((unsigned long long)fileTime.dwHighDateTime << 32) | (unsigned long long)fileTime.dwLowDateTime;
    time_t seconds = (microsecond - EPOCH_DIFF) / 10000000;
    return seconds;
}

/**
 * @brief �Ӹ���·���͸��ڵ㿪ʼ�ݹ鹹��Ŀ¼����
 * @param parentNode ���ڹ���Ŀ¼���ĸ��ڵ�ָ�롣
 * @param path ��ʼ����Ŀ¼����Ŀ¼·����
 */
void buildTree(TreeNode* parentNode, const char* path) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(path, &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error finding file/directory\n");
        return;
    }

    // ʹ��ջ��ģ��ݹ���ù���
    stack<pair<TreeNode*, string>> stack;
    stack.push({ parentNode, path });

    while (!stack.empty()) {
        // ȡ��ջ��Ԫ�أ����д���
        auto current = stack.top();
        stack.pop(); 

        //printf("Processing path: %s\n", current.second.c_str());  // �����ǰ�����·��

        HANDLE hFind = FindFirstFileA(current.second.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            continue;
        }

        do {
            // ���Ե�ǰĿ¼�͸�Ŀ¼
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
                continue;
            }

            // �����½ڵ㲢��������
            TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
            // ���·��ĩβ��"\*"����ȥ���ⲿ������
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

            // ���ڵ���ӵ���ϣ����
            addToIndex(newNode);

            //printf("Adding node: %s, isDirectory: %d, size: %d\n", newNode->name, newNode->isDirectory, newNode->size);  // �����ӵĽڵ���Ϣ

            // ���½ڵ����Ϊ�丸�ڵ���ӽڵ�
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

            // �����Ŀ¼��������Ŀ¼��ջ����������
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
 * @brief ����Ŀ¼������ȡ�
 * @param root Ŀ¼���ĸ��ڵ�ָ�롣
 * @return Ŀ¼���������ȡ�
 */
int calculateTreeDepth(TreeNode* root) {
    if (root == nullptr) {
        return 0;
    }

    queue<TreeNode*> q;
    q.push(root);
    int depth = 0;

    while (!q.empty()) {
        int size = q.size();   // ��ǰ��Ľڵ���

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
        ++depth;  // ÿ������һ�㣬��ȼ�һ
    }
    return depth;
}

/**
 * @brief �ݹ��ͷ�Ŀ¼���еĽڵ㼰�������ӽڵ�ռ�õ��ڴ�
 * @param root Ŀ¼���ĸ��ڵ�ָ��
 * @note �˺�����Ŀ¼�����к����������֤���ͷŸ��ڵ�֮ǰ�ӽڵ��Ѿ����ͷ�
 */
void freeTree(TreeNode* root) {
    if (root == NULL) {
        return;
    }

    TreeNode* currentChild = root->child;
    while (currentChild != NULL) {
        TreeNode* nextChild = currentChild->sibling;
        freeTree(currentChild);   // ���ȵݹ��ͷ��ӽڵ�
        free(currentChild);       // Ȼ���ͷŵ�ǰ�ڵ�
        currentChild = nextChild; // �ƶ����ֵܽڵ�
    }
}

/**
 * @brief ��鲢��ӡĿ¼���ṹ
 * @param node ��ǰ��Ҫ��ӡ��Ŀ¼���ڵ�
 * @param level ��Ӧ�Ĳ㼶�����ڴ�ӡ������
 * @note �˺���ʹ��ǰ������ķ�ʽ��ӡĿ¼��������ֱ�۵ز鿴���ṹ
 */
void printTree(TreeNode* node, int level) {
    if (node == NULL) {
        return;
    }

    for (int i = 0; i < level; i++) {
        printf("  "); // ��ӡ�ո���Ϊ����
    }
    // ��ӡ�ڵ���Ϣ���ڵ�������Ŀ¼�����ļ����ڵ��С
    printf("|- %s (%s, %d bytes)\n", node->name, node->isDirectory ? "Directory" : "File", node->size);

    // �ݹ��ӡ�ӽڵ���ֵܽڵ�
    printTree(node->child, level + 1); // �ӽڵ�㼶��һ
    printTree(node->sibling, level);   // �ֵܽڵ�㼶����
}