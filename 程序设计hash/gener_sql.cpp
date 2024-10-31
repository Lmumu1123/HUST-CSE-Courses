#include "all.h"

/**
 * @brief 将文件信息插入 SQL 文件中（含外键）
 * @param directory 文件的目录
 * @param file_path 文件的路径
 * @param file_size 文件的大小（单位：字节）
 * @param file_time 文件的最后修改时间（UNIX时间戳格式）
 * @param sqlFile 文件信息输出的 SQL 脚本文件流引用
 */
void EXinsertIntoSQLFile(const char* directory, char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile) {
    sqlFile << "insert into files(directory_id,filename,size,modified_time) values("
        << "(select id from directories where path='" << directory << "'),'"
        << file_path << "'," << file_size << ",from_unixtime(" << file_time << "));\n";
}

/**
 * @brief 将文件信息插入 SQL 文件中
 * @param file_path 文件的路径
 * @param file_size 文件的大小（单位：字节）
 * @param file_time 文件的最后修改时间（UNIX时间戳格式）
 * @param sqlFile 文件信息输出的 SQL 脚本文件流引用
 */
void insertIntoSQLFile(char* file_path, uintmax_t file_size, time_t file_time, ofstream& sqlFile) {
    sqlFile << "INSERT INTO files(filename, size, modified_time) VALUES ('"
        << file_path << "', " << file_size << ", FROM_UNIXTIME(" << file_time << "));\n";
}

/**
 * @brief 将目录信息插入 SQL 文件中
 * @param directory 扫描的根目录路径
 * @param subdirPath 子目录的路径
 * @param dirSqlFile 目录信息输出的 SQL 脚本文件流引用
 */
void insertDirIntoSQLFile(const string& directory, const string& subdirPath, ofstream& dirSqlFile) {
    dirSqlFile << "insert into directories(path) values('" << subdirPath << "');\n";
}

//数据库建表内容
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
//有外键
//CREATE TABLE if not EXISTS files(
//    id INT auto_increment PRIMARY KEY,
//    directory_id INT,
//    filename VARCHAR(255) NOT NULL,
//    size BIGINT NOT NULL,
//    modified_time TIMESTAMP NOT NULL,
//    CONSTRAINT fk_files_directories FOREIGN KEY(directory_id) REFERENCES directories(id)
//);
