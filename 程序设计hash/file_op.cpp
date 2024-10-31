#include "tree.h"
extern std::unordered_map<std::string, TreeNode*> nodeIndex;
extern TreeNode* rootNode;

/**
 * @brief �Ӹ��ڵ㿪ʼѰ��ָ���ڵ�ĸ��ڵ�
 * @param root Ŀ¼���ĸ��ڵ�
 * @param node ҪѰ�Ҹ��ڵ��Ŀ��ڵ�
 * @return ����Ŀ��ڵ�ĸ��ڵ�ָ�룬���������򷵻� NULL
 */
TreeNode* find_parent(TreeNode* root, TreeNode* node) {
    TreeNode* stack[MAX_STACK] = {};
    TreeNode* p;
    int top = -1;
    stack[++top] = root;
    // ʹ��ջ�����������Ҹ��ڵ�
    while (top > -1) {
        p = stack[top];
        if (p->child  == node || p->sibling == node) return p;
        top--;
        if (p->sibling) stack[++top] = p->sibling;
        if (p->child) stack[++top] = p->child;
    }
    return NULL; // û���ҵ����ڵ�
}

/**
 * @brief �޸�ָ���ļ���ʱ��ʹ�С����
 * @param directory �ļ���Ŀ¼·��
 * @param time �µ�ʱ������
 * @param size �µ��ļ���С
 */
void modify_f(string directory, time_t time, long size) {
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        //cerr << "�ļ������ڣ��޷��޸�" << endl;
        return ;
    }
    // ���½ڵ�ʱ��ʹ�С����
    node->time = time;
    node->size = size;
}

/**
 * @brief ɾ��ָ�����ļ��ڵ�
 * @param directory �ļ���Ŀ¼·��
 */
void delete_f(string directory) {
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        //cerr << "�ļ������ڣ��޷�ɾ��" << endl;
        return;
    }
    TreeNode* parent_node = node->parent;
    //TreeNode* parent_node = find_parent(rootNode,node);
    // ���¸��ڵ��ӽڵ������Ƴ���ǰ�ڵ�
    if (parent_node->child == node) {
        parent_node->child = node->sibling;
    }
    else {
        parent_node->sibling = node->sibling;
    }
    if (node->sibling) node->sibling->parent = parent_node;

    //���ڵ�ӹ�ϣ�����Ƴ�
    std::string indexKey = std::to_string(std::hash<std::string>{}(node->name));
    nodeIndex.erase(indexKey);
    // �ͷŽڵ��ڴ�
    delete node; 
}

/**
 * @brief ��ָ��Ŀ¼��������ļ��ڵ�
 * @param directory_add Ŀ¼��·��
 * @param file_name ���ļ�������
 * @param time �ļ���ʱ������
 * @param size �ļ��Ĵ�С
 */
void add_f(string directory_add, string file_name, time_t time, long size) {
    TreeNode* node = findNodeByPath(directory_add);
    if (!node) {
        //cerr << "�ļ�Ŀ¼�����ڣ��޷����" << endl;
        return;
    }
    // �������ļ��ڵ㲢��������
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    newNode->time = time;
    strcpy(newNode->name, file_name.c_str());
    newNode->isDirectory = 0;
    newNode->size = size;
    newNode->child = NULL;
    newNode->sibling = NULL;

    // ���½ڵ���ӵ�Ŀ¼��
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

    addToIndex(newNode); // ���½ڵ���ӵ�������

}

/**
 * @brief ���������ļ��е��ļ�����ָ�ִ����ӡ��޸ĺ�ɾ������
 * @param inputFileName �����ļ�����ָ��������ļ�·��
 */
void ProcessFile(const string& inputFileName) {
    ifstream infile(inputFileName);
    // �������ļ�������Ƿ�ɹ�
    if (!infile.is_open()) {
        cerr << "Error opening input or output file." << endl;
        return;
    }
    // ����Ƿ��ڶ�ȡ�ļ������Ĳ���
    bool is_reading_dirs = false;
    string line;
    // ���ж�ȡ�����ļ�
    while (getline(infile, line)) {
        //cout << "��ǰ�е�����Ϊ��" << line << endl;
        if (line == "selected files") {
            is_reading_dirs = true; // ��ʼ�����ļ�����
            continue;
        }
        else if (line == "end of files") {
            is_reading_dirs = false; // ���������ļ�����
            break;
        }
        // �����ǰ�����ļ���������׶�
        if (is_reading_dirs) {
            // ����Ŀ¼��Ϣ
            if (!line.empty()) {
                // �ֽ��ַ����Ի�ȡ�ļ���·��������ģʽ��ʱ��ʹ�С����Ϣ
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
                // ִ����Ӧ���ļ�����
                if (mode == "D") delete_f(directory);  // ɾ���ļ�����
                else {
                    time_t time = (time_t)stol(timestr);
                    long size = stol(sizestr);
                    if (mode == "M")  modify_f(directory, time, size); // �޸��ļ�����
                    else {
                        // ��ȡ����ļ���Ŀ¼����
                        string directory_add = directory.substr(start, directory.rfind('\\') - start);
                        add_f(directory_add, directory, time, size); // ����ļ�����
                    }
                }

            }
        }
    }
    // ��ɲ����󣬹ر��ļ�
    infile.close();
}

