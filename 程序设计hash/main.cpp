#include "tree.h"
#include "file.h"
#include "scanC.h"

// 交互界面展示目录
extern void showMenu1();
extern void showMenu2();

TreeNode* rootNode = (TreeNode*)malloc(sizeof(TreeNode)); // 将头节点定义为全局变量

int main() {
    char path[MAX_PATH];
    sprintf(path, "c:\\windows");
    string inputFileName = ".\\数据文件\\mystat.txt";
    string outputFileName1 = ".\\数据文件\\output1.txt";
    string outputFileName2 = ".\\数据文件\\output2.txt";
    string outputFileName3 = ".\\数据文件\\output3.txt";

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

    std::cout << "This is red text" << std::endl;

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // 恢复默认颜色


    bool flag = 0;
    while (!flag) {
        system("cls");
        showMenu1();
        int choose = 0;
        cin >> choose;
        switch (choose) {
        case 1: {
            // 磁盘文件属性获取和生成SQL语句文件
            CFileInfo* Cfiles = (CFileInfo*)malloc(sizeof(CFileInfo));

            // 扫描目录并获取文件信息
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

            cout << "\n# 磁盘目录/文件扫描完成，信息如下： #\n" << endl;
            printf("子目录数量: %d\n", Cfiles->numDirectories);
            printf("文件总数量: %d\n", Cfiles->numFiles);
            printf("目录层数: %d\n", Cfiles->maxDepth);
            printf("最长的带全路径的文件名及长度: %s，长度为 %d\n", Cfiles->longestPath, Cfiles->longestPathLength);
            cout << "\n# 生成插入数据库的SQL语句完成，可前往目标文件夹查询 #\n" << endl;
            system("pause");
            break;
        }
        case 2: {
            // 构造目录树
            strcpy(rootNode->name, "c:\\windows");
            rootNode->isDirectory = 1;
            rootNode->size = 0;
            rootNode->parent = NULL;
            rootNode->child = NULL;
            rootNode->sibling = NULL;

            char searchPath[MAX_PATH];
            snprintf(searchPath, MAX_PATH, "%s\\*", path);
            buildTree(rootNode, searchPath);
            cout << "\n# 构建目录树成功！ #\n" << endl;
            //printTree(rootNode, 0);  //查看孩子兄弟树的结构
            // 计算目录树深度
            int depth = calculateTreeDepth(rootNode);
            printf("目录树的深度: %d\n", depth);
            system("pause");
            break;
        }
        case 3: {
            // 记录程序开始时间
            auto start = std::chrono::high_resolution_clock::now();
            //第一次统计文件信息
            ProcessDirectories(inputFileName, outputFileName1);
            cout << "\n# 第一次统计文件信息完成 #\n" << endl;
            // 记录程序结束时间
            auto end = std::chrono::high_resolution_clock::now();

            // 计算程序的运行时间
            std::chrono::duration<double> duration = end - start;
            std::cout << "程序运行时间为: " << duration.count() << " 秒" << std::endl;
            system("pause");
            break;
        }
        case 4: {
            // 记录程序开始时间
            auto start = std::chrono::high_resolution_clock::now();
            //模拟文件操作(myfile.txt),并展示3条操作后的文件信息变化。
            cout << "查看第二次统计信息和差异（任选3条）：" << endl;
            string file1 = "c:\\windows\\Setup\\State\\State.ini";
            string file2 = "c:\\windows\\SystemApps\\MicrosoftWindows.Client.CBS_cw5n1h2txyewy\\WebExperienceHost.dll";
            string file3 = "c:\\windows\\SystemApps\\Microsoft.ECApp_8wekyb3d8bbwe\\GazeInteraction.dll2401";
            cout << "操作前文件信息" << endl;
            Print(file1);
            cout << '\n';
            Print(file2);
            cout << '\n';
            Print(file3);
            ProcessFile(".\\数据文件\\myfile.txt");
            cout << "\n# 模拟文件操作完成 #\n" << endl;
            cout << "操作后文件信息" << endl;
            Print(file1);
            cout << '\n';
            Print(file2);
            cout << '\n';
            Print(file3);
            //第二次统计文件信息
            ProcessDirectories(inputFileName, outputFileName2);
            cout << "\n# 第二次统计文件信息完成 #\n" << endl;
            // 记录程序结束时间
            auto end = std::chrono::high_resolution_clock::now();

            // 计算程序的运行时间
            std::chrono::duration<double> duration = end - start;
            std::cout << "程序运行时间为: " << duration.count() << " 秒" << std::endl;
            system("pause");
            break;
        }
        case 5: {
            // 记录程序开始时间
            auto start = std::chrono::high_resolution_clock::now();
            //模拟目录操作(mydir.txt)，并展示一条目录和下层子目录的文件信息变化。
            cout << "查看第三次统计信息和差异（任选1条）：" << endl;
            cout << "选择输出的目录为c:\\windows\\Microsoft.NET\\assembly\\GAC_MSIL\\ \n" << endl;
            string file4 = "c:\\windows\\Microsoft.NET\\assembly\\GAC_MSIL\\Accessibility\\v4.0_4.0.0.0__b03f5f7f11d50a3a\\Accessibility.dll";
            cout << "操作前文件信息" << endl;
            Print(file4);
            ProcessDir(".\\数据文件\\mydir.txt");
            cout << "\n# 模拟目录操作完成 #\n" << endl;
            cout << "操作后文件信息" << endl;
            Print(file4);
            //第三次统计文件信息
            ProcessDirectories(inputFileName, outputFileName3);
            cout << "\n# 第三次统计文件信息完成 #\n" << endl;
            // 记录程序结束时间
            auto end = std::chrono::high_resolution_clock::now();

            // 计算程序的运行时间
            std::chrono::duration<double> duration = end - start;
            std::cout << "程序运行时间为: " << duration.count() << " 秒" << std::endl;
            system("pause");
            break;
        }
        case 6: {
            // 查看统计信息变A化 
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

