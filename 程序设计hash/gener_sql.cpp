#include "all.h"

/**
 * @brief ���ļ���Ϣ���� SQL �ļ��У��������
 * @param directory �ļ���Ŀ¼
 * @param file_path �ļ���·��
 * @param file_size �ļ��Ĵ�С����λ���ֽڣ�
 * @param file_time �ļ�������޸�ʱ�䣨UNIXʱ�����ʽ��
 * @param sqlFile �ļ���Ϣ����� SQL �ű��ļ�������
 */
void EXinsertIntoSQLFile(const char* directory, char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile) {
    sqlFile << "insert into files(directory_id,filename,size,modified_time) values("
        << "(select id from directories where path='" << directory << "'),'"
        << file_path << "'," << file_size << ",from_unixtime(" << file_time << "));\n";
}

/**
 * @brief ���ļ���Ϣ���� SQL �ļ���
 * @param file_path �ļ���·��
 * @param file_size �ļ��Ĵ�С����λ���ֽڣ�
 * @param file_time �ļ�������޸�ʱ�䣨UNIXʱ�����ʽ��
 * @param sqlFile �ļ���Ϣ����� SQL �ű��ļ�������
 */
void insertIntoSQLFile(char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile) {
    sqlFile << "INSERT INTO files(filename, size, modified_time) VALUES ('"
        << file_path << "', " << file_size << ", FROM_UNIXTIME(" << file_time << "));\n";
}

/**
 * @brief ��Ŀ¼��Ϣ���� SQL �ļ���
 * @param directory ɨ��ĸ�Ŀ¼·��
 * @param subdirPath ��Ŀ¼��·��
 * @param dirSqlFile Ŀ¼��Ϣ����� SQL �ű��ļ�������
 */
void insertDirIntoSQLFile(const string& directory, const string& subdirPath, ofstream& dirSqlFile) {
    dirSqlFile << "insert into directories(path) values('" << subdirPath << "');\n";
}

//���ݿ⽨������
//CREATE TABLE if not EXISTS directories(
//	id INT auto_increment PRIMARY KEY,
//	path VARCHAR(255) NOT NULL
//);
//
//CREATE TABLE if not EXISTS files(
//	id INT auto_increment PRIMARY KEY,
//	filename VARCHAR(255) NOT NULL,
//	size BIGINT NOT NULL,
//	modified_time TIMESTAMP NOT NULL
//	);
//�����
//CREATE TABLE if not EXISTS files(
//    id INT auto_increment PRIMARY KEY,
//    directory_id INT,
//    filename VARCHAR(255) NOT NULL,
//    size BIGINT NOT NULL,
//    modified_time TIMESTAMP NOT NULL,
//    CONSTRAINT fk_files_directories FOREIGN KEY(directory_id) REFERENCES directories(id)
//);
