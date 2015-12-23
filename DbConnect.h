/*
 * db_mysql.h
 *
 *  Created on: 2015年9月17日
 *      Author: root
 */

#ifndef DB_CONNECT_H_
#define DB_CONNECT_H_

extern "C" {
#include "mysql/mysql.h"
}
#include "DbRecordset.h"
#include <string>

class DbConnect
{
public:
	DbConnect();

	~DbConnect();

public:
	void open(const char* ip, int port,
			const char* db, int timeout,
			const char* usrname, const char* password);

	//关闭数据库连接
	void close();

	//获取错误码
	int get_error_no();

	//获取错误原因
	const char* get_error_info();

	//打印错误信息
	void print_error();

	//连接数据库
	bool connect();

	//检测数据库连接是否正常
	bool ensure_connected();

	//设置提交模式
	void set_commit_mode();

	//事务开始
	void transaction();

	//提交事务
	void commit();

	//回滚事务
	void rollback();

	//读锁定表
	bool lock_table_read(const char* psTableName);

	//写锁定表
	bool lock_table_write(const char* psTableName);

	//解除表锁定
	bool unlock_table(const char* psTableName);

	
	//执行查询命令
	DbRecordset* query_record(
						const char* table,
						const char* columns,
						const char* condition,
						const char* sort,
						const char* limit);
					
	DbRecordset*  do_mysql_cmd(const char* cmd);

	//执行插入命令
	bool insert_record(
						 const char* table,
						 const char* columns,
						 const char* values);

	//执行更新命令
	bool update_record(
						 const char* table,
						 const char* condition,
						 const char* fmt, ...);

	DbRecordset* exec_proc_record(
						const char* procedure_name,
						const char* fmt, ...);

	bool exec_proc(
						const char* procedure_name,
						const char* fmt, ...);
						
	

private:
	MYSQL* _mysql_handle;
	DbRecordset _record_set;

	std::string _ip;
	int _port;
	std::string _db;
	std::string _username;
	std::string _password;

};


#endif /* DB_CONNECT_H_ */
