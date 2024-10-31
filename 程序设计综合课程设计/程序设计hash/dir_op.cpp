#include "tree.h"
extern std::unordered_map<std::string, TreeNode*> nodeIndex;
extern TreeNode* rootNode;

/**
 * @brief �ݹ���ͷ�Ŀ¼���ڵ㼰���ӽڵ���ڴ棬���ӹ�ϣ����ɾ����Ӧ��
 * @param root Ŀ¼���ĸ��ڵ㣬���Ӵ˽ڵ㿪ʼ�ͷ�
 */
void freeDir(TreeNode* root) {
    if (root == NULL) {
        return;     // ����ڵ�Ϊ�գ���ֱ�ӷ���
    }

    // ���������ӽڵ㲢�ݹ��ͷ�
    TreeNode* currentChild = root->child;
    while (currentChild != NULL) {
        TreeNode* nextChild = currentChild->sibling;
        freeDir(currentChild); // �ݹ��ͷ��ӽڵ�

        // ���ڵ����ƴӹ�ϣ����ɾ��
        std::string indexKey = std::to_string(std::hash<std::string>{}(currentChild->name));
        nodeIndex.erase(indexKey);

        free(currentChild); // �ͷŵ�ǰ�ڵ���ڴ�
        currentChild = nextChild; // ���������ֵܽڵ�
    }
}

/**
 * @brief ɾ��ָ����Ŀ¼���������ӽڵ�
 * @param directory Ҫɾ����Ŀ¼��·��
 * @note �ú��������Ŀ¼�ڵ�ĸ��ӹ�ϵ�������ͷ�������ؽڵ���ڴ�
 */
void delete_d(string directory) {
    // ͨ��·���ҵ�ָ���Ľڵ�
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        //cerr << "�ļ������ڣ��޷�ɾ��" << endl;
        return;
    }
    // ���¸��ڵ���ӽڵ����������Ƴ���ǰ�ڵ�
    TreeNode* parent_node = node->parent;
    //TreeNode* parent_node = find_parent(rootNode,node);
    if (parent_node->child == node) {
        parent_node->child = node->sibling;
    }
    else {
        parent_node->sibling = node->sibling;
    }
    // ������ֵܽڵ㣬�����ֵܽڵ�ĸ�ָ��
    if (node->sibling) node->sibling->parent = parent_node;

    // �ͷŸ�Ŀ¼���������ӽڵ���ڴ�
    freeDir(node);
    // ���ڵ�ӹ�ϣ�����Ƴ�
    std::string indexKey = std::to_string(std::hash<std::string>{}(node->name));
    nodeIndex.erase(indexKey);
    // ɾ���ڵ�
    delete node;

}

/**
 * @brief ���������ļ����оٵ�����Ŀ¼����ִ��ɾ������
 * @param inputFileName �����ļ������ƣ����ļ�����Ҫɾ����Ŀ¼�б�
 * @note �ļ���ÿһ�б�ʾһ��Ҫɾ����Ŀ¼·��
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

    // ���ж�ȡ�����ļ�
    while (getline(infile, line)) {
        //cout << "��ǰ�е�����Ϊ��" << line << endl;

        if (line == "selected dirs") {
            is_reading_dirs = true;
            continue;
        }
        else if (line == "end of dirs") {
            is_reading_dirs = false;
            break;
        }

        if (is_reading_dirs) {
            // ����Ŀ¼��Ϣ
            if (!line.empty()) {
                size_t start = line.find("c:\\");
                size_t partition1 = line.find(",");//��һ������

                string directory = line.substr(start, partition1 - start - 1);

                //cout << directory << endl;
                 // ִ��ɾ��Ŀ¼����
                delete_d(directory);
            }
        }
    }
    infile.close();// �ر������ļ�
}

