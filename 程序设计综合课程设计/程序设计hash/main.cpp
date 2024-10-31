#include "tree.h"
#include "file.h"
#include "scanC.h"

// ��������չʾĿ¼
extern void showMenu1();
extern void showMenu2();

TreeNode* rootNode = (TreeNode*)malloc(sizeof(TreeNode)); // ��ͷ�ڵ㶨��Ϊȫ�ֱ���

int main() {
    char path[MAX_PATH];
    sprintf(path, "c:\\windows");
    string inputFileName = ".\\�����ļ�\\mystat.txt";
    string outputFileName1 = ".\\�����ļ�\\output1.txt";
    string outputFileName2 = ".\\�����ļ�\\output2.txt";
    string outputFileName3 = ".\\�����ļ�\\output3.txt";

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

    std::cout << "This is red text" << std::endl;

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // �ָ�Ĭ����ɫ


    bool flag = 0;
    while (!flag) {
        system("cls");
        showMenu1();
        int choose = 0;
        cin >> choose;
        switch (choose) {
        case 1: {
            // �����ļ����Ի�ȡ������SQL����ļ�
            CFileInfo* Cfiles = (CFileInfo*)malloc(sizeof(CFileInfo));

            // ɨ��Ŀ¼����ȡ�ļ���Ϣ
            ofstream SQLfile("./sql_file/file_1.sql");
            if (!SQLfile.is_open()) {
                cerr << "Error: Unable to open file SQL file" << endl;
                SQLfile.close();
            }
            ofstream SQLdir("./sql_dir/dir_1.sql");
            if (!SQLdir.is_open()) {
                cerr << "Error: Unable to open directory SQL file" << endl;
                SQLdir.close();
            }
       
            listCFiles("c:/windows", Cfiles, SQLfile, SQLdir);

            cout << "\n# ����Ŀ¼/�ļ�ɨ����ɣ���Ϣ���£� #\n" << endl;
            printf("��Ŀ¼����: %d\n", Cfiles->numDirectories);
            printf("�ļ�������: %d\n", Cfiles->numFiles);
            printf("Ŀ¼����: %d\n", Cfiles->maxDepth);
            printf("��Ĵ�ȫ·�����ļ���������: %s������Ϊ %d\n", Cfiles->longestPath, Cfiles->longestPathLength);
            cout << "\n# ���ɲ������ݿ��SQL�����ɣ���ǰ��Ŀ���ļ��в�ѯ #\n" << endl;
            system("pause");
            break;
        }
        case 2: {
            // ����Ŀ¼��
            strcpy(rootNode->name, "c:\\windows");
            rootNode->isDirectory = 1;
            rootNode->size = 0;
            rootNode->parent = NULL;
            rootNode->child = NULL;
            rootNode->sibling = NULL;

            char searchPath[MAX_PATH];
            snprintf(searchPath, MAX_PATH, "%s\\*", path);
            buildTree(rootNode, searchPath);
            cout << "\n# ����Ŀ¼���ɹ��� #\n" << endl;
            //printTree(rootNode, 0);  //�鿴�����ֵ����Ľṹ
            // ����Ŀ¼�����
            int depth = calculateTreeDepth(rootNode);
            printf("Ŀ¼�������: %d\n", depth);
            system("pause");
            break;
        }
        case 3: {
            // ��¼����ʼʱ��
            auto start = std::chrono::high_resolution_clock::now();
            //��һ��ͳ���ļ���Ϣ
            ProcessDirectories(inputFileName, outputFileName1);
            cout << "\n# ��һ��ͳ���ļ���Ϣ��� #\n" << endl;
            // ��¼�������ʱ��
            auto end = std::chrono::high_resolution_clock::now();

            // ������������ʱ��
            std::chrono::duration<double> duration = end - start;
            std::cout << "��������ʱ��Ϊ: " << duration.count() << " ��" << std::endl;
            system("pause");
            break;
        }
        case 4: {
            // ��¼����ʼʱ��
            auto start = std::chrono::high_resolution_clock::now();
            //ģ���ļ�����(myfile.txt),��չʾ3����������ļ���Ϣ�仯��
            cout << "�鿴�ڶ���ͳ����Ϣ�Ͳ��죨��ѡ3������" << endl;
            string file1 = "c:\\windows\\Setup\\State\\State.ini";
            string file2 = "c:\\windows\\SystemApps\\MicrosoftWindows.Client.CBS_cw5n1h2txyewy\\WebExperienceHost.dll";
            string file3 = "c:\\windows\\SystemApps\\Microsoft.ECApp_8wekyb3d8bbwe\\GazeInteraction.dll2401";
            cout << "����ǰ�ļ���Ϣ" << endl;
            Print(file1);
            cout << '\n';
            Print(file2);
            cout << '\n';
            Print(file3);
            ProcessFile(".\\�����ļ�\\myfile.txt");
            cout << "\n# ģ���ļ�������� #\n" << endl;
            cout << "�������ļ���Ϣ" << endl;
            Print(file1);
            cout << '\n';
            Print(file2);
            cout << '\n';
            Print(file3);
            //�ڶ���ͳ���ļ���Ϣ
            ProcessDirectories(inputFileName, outputFileName2);
            cout << "\n# �ڶ���ͳ���ļ���Ϣ��� #\n" << endl;
            // ��¼�������ʱ��
            auto end = std::chrono::high_resolution_clock::now();

            // ������������ʱ��
            std::chrono::duration<double> duration = end - start;
            std::cout << "��������ʱ��Ϊ: " << duration.count() << " ��" << std::endl;
            system("pause");
            break;
        }
        case 5: {
            // ��¼����ʼʱ��
            auto start = std::chrono::high_resolution_clock::now();
            //ģ��Ŀ¼����(mydir.txt)����չʾһ��Ŀ¼���²���Ŀ¼���ļ���Ϣ�仯��
            cout << "�鿴������ͳ����Ϣ�Ͳ��죨��ѡ1������" << endl;
            cout << "ѡ�������Ŀ¼Ϊc:\\windows\\Microsoft.NET\\assembly\\GAC_MSIL\\ \n" << endl;
            string file4 = "c:\\windows\\Microsoft.NET\\assembly\\GAC_MSIL\\Accessibility\\v4.0_4.0.0.0__b03f5f7f11d50a3a\\Accessibility.dll";
            cout << "����ǰ�ļ���Ϣ" << endl;
            Print(file4);
            ProcessDir(".\\�����ļ�\\mydir.txt");
            cout << "\n# ģ��Ŀ¼������� #\n" << endl;
            cout << "�������ļ���Ϣ" << endl;
            Print(file4);
            //������ͳ���ļ���Ϣ
            ProcessDirectories(inputFileName, outputFileName3);
            cout << "\n# ������ͳ���ļ���Ϣ��� #\n" << endl;
            // ��¼�������ʱ��
            auto end = std::chrono::high_resolution_clock::now();

            // ������������ʱ��
            std::chrono::duration<double> duration = end - start;
            std::cout << "��������ʱ��Ϊ: " << duration.count() << " ��" << std::endl;
            system("pause");
            break;
        }
        case 6: {
            // �鿴ͳ����Ϣ��A�� 
            bool sign = 0;
            while (!sign) {
                system("cls");
                showMenu2();
                int check = 0;
                cin >> check;

                switch (check) {
                case 1: {
                    compareFiles(outputFileName1, outputFileName2);
                    system("pause");
                    break;
                }
                case 2: {
                    compareFiles(outputFileName2, outputFileName3);
                    system("pause");
                    break;
                }
                case 3: {
                    compareFiles(outputFileName1, outputFileName3);
                    system("pause");
                    break;
                }
                default: {
                    sign = 1;
                    break;
                }
                }
            }

            break;
        }
        default: {
            flag = 1;
            break;
        }
        }

    }

    //freeTree(rootNode);
    //free(rootNode);

    return 0;
}

