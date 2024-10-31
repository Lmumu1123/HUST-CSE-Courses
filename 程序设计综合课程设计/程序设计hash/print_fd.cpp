#include "tree.h"

/**
 * @brief ����ת��Ϊ����ʱ���ʽ���ַ���
 * @param seconds UNIXʱ������룩
 * @param dateTimeString ����������洢ת���������ʱ���ַ���
 * @return ����ָ��dateTimeString��ָ�룬���ת��ʧ����ΪNULL
 */
char* Microsecond_To_DateTime(time_t seconds, char* dateTimeString) {
    struct tm* dateTimeInfo = localtime(&seconds);
    if (dateTimeInfo != NULL) {
        // ��ʽ������ʱ�䲢�洢�� dateTimeString ��
        strftime(dateTimeString, 100, "%Y��%m��%d�� %H:%M:%S", dateTimeInfo);
        return dateTimeString;
    }
    else {
        printf("����ת��������ʱ�����\n");
        return NULL;  // ת��ʧ�ܷ���NULL
    }
}

/**
 * @brief ��ӡָ��Ŀ¼����Ϣ
 * @param directory Ҫ��ӡ��Ϣ��Ŀ¼·��
 * @note �˺�����ӡĿ¼���޸�ʱ��ʹ�С
 */
void Print(string directory) {
    TreeNode* node = findNodeByPath(directory);
    if (!node) {
        cout << "δ�ҵ�" << directory << " ����Ϣ" << endl;
        return;
    }
    char dateTimeString[100] = "";
    // ת��ʱ�䲢��ӡ�ڵ���Ϣ
    if (Microsecond_To_DateTime(node->time, dateTimeString)) {
        printf("%s ��ϢΪ:\nʱ�䣺%s ��С��%ldB\n", node->name, dateTimeString, node->size);
    }
    else {
        printf("ʱ��ת������\n");
    }
}