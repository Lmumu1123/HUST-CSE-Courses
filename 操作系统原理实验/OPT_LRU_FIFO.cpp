#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <stack>
#include <map>
#include <queue>

using namespace std;

#define MAX_NUM 10000
const int pageSize = 10;      // 页面大小
const int instrCnt = MAX_NUM; // 指令数目
const int pageFrameCnt = 3;   // 页框数

int orderCnt;                 // 访问次数
int cnt = 0;                  // 当前页框数
int LRUpageMissCnt = 0;       // LRU 缺页数
int OPTpageMissCnt = 0;       // OPT 缺页数
int FIFOpagMissCnt = 0;       // FIFO 缺页数
int op;                        // 选择页替换算法
int processArr[instrCnt];     // 进程数组
int pageFrame[pageFrameCnt][pageSize]; // 页框数组
int orderArr[MAX_NUM];        // 指令访问序列
int pageIdx[pageFrameCnt];    // 页框中存的页号
int arraykind;                // 访问数组类型，0为随机，1为顺序，2为循环

// 初始化进程
inline void initProcess() {
    for (int i = 0; i < instrCnt; ++i) {
        processArr[i] = rand() % 1000; // 随机生成进程数据
    }
}

// 初始化访问序列
void initInstrOrder(int kind, int size) {
    orderCnt = size;
    if (kind == 1) {
        for (int i = 0; i < orderCnt; ++i) {
            orderArr[i] = rand() % size; // 随机访问
        }
    } else {
        for (int i = 0; i < orderCnt; ++i) {
            orderArr[i] = i; // 顺序访问
        }
    }
}

// 将页号为 pageNo 页面的数据复制到第 pfIdx 的页框中
inline void copyPage(int pfIdx, int pageNo) {
    memcpy(pageFrame[pfIdx], processArr + pageNo * pageSize, pageSize * sizeof(int));
    pageIdx[pfIdx] = pageNo; // 记录页号
}

// LRU算法实现
void LRU() {
    int timer[pageFrameCnt];
    memset(timer, 0, sizeof(timer));
    for (int i = 0; i < orderCnt; ++i) {
        auto pageNo = orderArr[i] / pageSize; // 页号
        auto offset = orderArr[i] % pageSize; // 页内偏移
        int j;

        // 命中
        for (j = 0; j < cnt; ++j) {
            if (pageIdx[j] == pageNo) {
                timer[j] = 0; // 更新使用时间
                break;
            }
        }
        // 未命中
        if (j == cnt) {
            ++LRUpageMissCnt;
            if (cnt == pageFrameCnt) {
                auto maxT = 0;
                for (int k = 0; k < pageFrameCnt; ++k) {
                    if (timer[k] > timer[maxT])
                        maxT = k;
                }
                copyPage(maxT, pageNo); // 替换页
                timer[maxT] = 0; // 重置时间
            } else {
                copyPage(cnt, pageNo); // 直接添加新页面
                ++cnt;
            }
        }
        for (int j = 0; j < cnt; ++j)
            ++timer[j];
    }
    cout << "visit times: " << orderCnt << " miss times: " << LRUpageMissCnt
         << " LRU miss rate: " << float(LRUpageMissCnt) / orderCnt * 100 << "%" << endl;
}

// FIFO算法实现
void FIFO() {
    queue<int> fifoQueue; // 使用队列来实现 FIFO 页替换算法
    for (int i = 0; i < orderCnt; ++i) {
        auto pageNo = orderArr[i] / pageSize; // 页号
        int j;

        // 查找当前页号在页框中
        for (j = 0; j < cnt; ++j) {
            if (pageIdx[j] == pageNo) {
                break; // 命中
            }
        }

        // 如果未命中
        if (j == cnt) {
            ++FIFOpagMissCnt; // 缺页次数+1

            // 如果页框已满
            if (cnt == pageFrameCnt) {
                int oldPage = fifoQueue.front(); // 获取最旧的页面
                fifoQueue.pop(); // 移除最旧的页面

                // 查找该页面在页框中的索引并替换
                for (int k = 0; k < cnt; ++k) {
                    if (pageIdx[k] == oldPage) {
                        copyPage(k, pageNo); // 替换为新页面
                        break;
                    }
                }
            } else {
                copyPage(cnt, pageNo); // 直接添加新页面
                ++cnt;
            }
            fifoQueue.push(pageNo); // 将新页面入队
        }
    }
    cout << "visit times: " << orderCnt << " miss times: " << FIFOpagMissCnt
         << " FIFO miss rate: " << float(FIFOpagMissCnt) / orderCnt * 100 << "%" << endl;
}

// OPT算法实现
void OPT() {
    map<int, stack<int>> ms;
    for (int i = orderCnt - 1; i >= 0; --i) {
        auto pageNo = orderArr[i] / pageSize;
        if (ms.count(pageNo) == 0) {
            stack<int> tmp;
            tmp.push(i);
            ms.insert(pair<int, stack<int>>(pageNo, tmp));
        } else {
            ms.at(pageNo).push(i);
        }
    }
    for (int i = 0; i < orderCnt; ++i) {
        auto pageNo = orderArr[i] / pageSize;
        int j;
        if (ms.at(pageNo).size())
            ms.at(pageNo).pop();
        for (j = 0; j < cnt; ++j) {
            if (pageIdx[j] == pageNo) {
                break; // 命中
            }
        }
        if (j == cnt) {
            ++OPTpageMissCnt; // 缺页次数+1
            if (cnt == pageFrameCnt) {
                auto maxT = 0;
                for (int k = 0; k < pageFrameCnt; ++k) {
                    if (ms.at(pageIdx[k]).size() == 0) {
                        maxT = k; // 找到不再使用的页
                        break;
                    } else if (ms.at(pageIdx[k]).top() > ms.at(pageIdx[maxT]).top()) {
                        maxT = k; // 找到最长时间不使用的页
                    }
                }
                copyPage(maxT, pageNo); // 替换页
            } else {
                copyPage(cnt, pageNo); // 添加新页
                ++cnt;
            }
        }
    }
    cout << "visit times: " << orderCnt << " miss times: " << OPTpageMissCnt
         << " OPT miss rate: " << float(OPTpageMissCnt) / orderCnt * 100 << "%" << endl;
}



// 主函数
int main() {
    srand(time(nullptr));
    printf("array size:\n");
    cin >> orderCnt; // 用户输入访问次数
    while (1) {
        initProcess(); // 初始化进程
        printf("select array kind:\n1.Random\n2.Sequence\n");
        cin >> arraykind; // 选择访问方式
        initInstrOrder(arraykind, orderCnt); // 初始化访问序列

        printf("1.OPT\n2.FIFO\n3.LRU\n4.quit\n");
        cin >> op; // 选择页替换算法
        if (op == 1) {
            OPTpageMissCnt = 0; // 重置缺页计数
            OPT(); // 执行 OPT 算法
        } else if (op == 3) {
            LRUpageMissCnt = 0; // 重置 LRU 缺页计数
            LRU(); // 执行 LRU 算法
        } else if (op == 2) {
            FIFOpagMissCnt = 0; // 重置 FIFO 缺页计数
            FIFO(); // 执行 FIFO 算法
        } else if (op == 4) {
            break; // 选择退出，结束程序
        } else {
            cout << "无效的选择，请重新输入。\n"; // 无效选择提示
        }
    }
    system("pause");
    return 0;
}