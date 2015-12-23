/*
 * DbConnect.cpp
 *
 *  Created on: 2015年9月17日
 *      Author: root
 */


#include "DbConnect.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

DbConnect::DbConnect() : _mysql_handle(NULL)
{
	_mysql_handle = new MYSQL;
	mysql_init(_mysql_handle);
	
}

DbConnect::~DbConnect()
{
	delete _mysql_handle;
}

//初始化数据库连接
void DbConnect::open(const char* ip, int port,
		const char* db, int timeout,
		const char* username, const char* password)
{
	if(NULL == _mysql_handle)
		return;

	_ip = ip;
	_port = port;
	_db = db;
	_username = username;
	_password = password;

	//设置链接超时时间.
	mysql_options(_mysql_handle, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&timeout);

	//设置查询数据库(select)超时时间
	mysql_options(_mysql_handle, MYSQL_OPT_READ_TIMEOUT, (const char *)&timeout);

	//设置写数据库(update,delect,insert,replace等)的超时时间。
	mysql_options(_mysql_handle, MYSQL_OPT_WRITE_TIMEOUT, (const char *)&timeout);
	
	mysql_options(_mysql_handle,MYSQL_SET_CHARSET_NAME,"utf8");

	connect();
}

//关闭数据库连接
void DbConnect::close()
{
	if(NULL == _mysql_handle)
		return ;

	_record_set.free_record();
	mysql_close(_mysql_handle);
	//mysql_thread_end();//用于多线程
}

//获取错误码
int DbConnect::get_error_no()
{
	if(NULL == _mysql_handle)
		return -1;

	return mysql_errno(_mysql_handle);
}

//获取错误原因
const char* DbConnect::get_error_info()
{
	if(NULL == _mysql_handle)
		return "";

	return mysql_error(_mysql_handle);
}

//打印错误信息
void DbConnect::print_error()
{
	if(NULL == _mysql_handle)
		return ;

	printf("errorno:%d, errorinfo:%s\n", get_error_no(), get_error_info());
}

//连接数据库
bool DbConnect::connect()
{
	if(NULL == _mysql_handle)
		return false;
	
	
	//mysql_thread_init();  //为多线程线程安全使用的
	//CLIENT_MULTI_STATEMENTS:通知服务器，客户端可能在单个字符串内发送多条语句（由‘;’隔开）。如果未设置该标志，将禁止多语句执行。
	MYSQL* pmysql = mysql_real_connect(_mysql_handle, _ip.c_str(), _username.c_str(), _password.c_str(),
			_db.c_str(), _port, 0, CLIENT_MULTI_STATEMENTS);
	if(NULL == pmysql)
	{
		print_error();
		return false;
	}

	mysql_set_character_set(_mysql_handle, "utf8");
	
	return true;
}

//检测数据库连接是否正常
bool DbConnect::ensure_connected()
{
	if(NULL == _mysql_handle)
		return false;

	if(0 != mysql_ping(_mysql_handle))
		return connect();

	return true;
}



//设置提交模式
void DbConnect::set_commit_mode()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_autocommit(_mysql_handle, 1);
}

//事务开始
void DbConnect::transaction()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_query(_mysql_handle, "start transaction");
}

//提交事务
void DbConnect::commit()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_commit(_mysql_handle);
}

//回滚事务
void DbConnect::rollback()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_rollback(_mysql_handle);
}

//读锁定表
bool DbConnect::lock_table_read(const char* psTableName)
{
	if(NULL == _mysql_handle)
		return -1;

	char strquery[1024] = {0};
	sprintf(strquery, "LOCK TABLES %s READ", psTableName);

	if(0 != mysql_query(_mysql_handle, strquery))
	{
		print_error();
		return -2;
	}
	return 0;
}

//写锁定表
bool DbConnect::lock_table_write(const char* psTableName)
{
	if(NULL == _mysql_handle)
		return -1;

	char strquery[254] = {0};
	sprintf(strquery, "LOCK TABLES %s WRITE", psTableName);

	if(0 != mysql_query(_mysql_handle, strquery))
	{
		print_error();
		return -2;
	}
	return 0;
}

//解除表锁定
bool DbConnect::unlock_table(const char* psTableName)
{
	if(NULL == _mysql_handle)
		return -1;

	if(false == ensure_connected())
		return 1;

	char strquery[254] = {0};
	sprintf(strquery, "UNLOCK TABLES %s", psTableName);
	if(0 != mysql_query(_mysql_handle, strquery))
	{
		print_error();
		return false;
	}

	return true;
}

//执行查询命令
DbRecordset* DbConnect::query_record(
					const char* table,
					const char* columns,
					const char* condition,
					const char* sort,
					const char* limit)
{
	if(NULL == table)
		return NULL;

	_record_set.free_record();

	char sql[512] = {0};
	strcat(sql, "select ");
	if(NULL == columns)
	{
		columns = "*";
	}

	strcat(sql, columns);
	strcat(sql, " from ");
	strcat(sql, table);

	if(NULL != condition)
	{
		strcat(sql, " where ");
		strcat(sql, condition);
	}
	if(NULL != sort)
	{
		strcat(sql, " order by ");
		strcat(sql, sort);
	}

	if(NULL != limit)
		strcat(sql, limit);

	if(0 != mysql_query(_mysql_handle, sql))
	{
		print_error();
		return NULL;
	}

	_record_set._res = mysql_store_result(_mysql_handle);
	if(NULL == _record_set._res)
	{
		print_error();
		return NULL;
	}

	return &_record_set;
}

//执行mysql的相关命令
DbRecordset*  DbConnect::do_mysql_cmd(const char* cmd)
{
	if(cmd == NULL ) return NULL;
	if(0 != mysql_query(_mysql_handle, cmd))
	{
		print_error();
		return NULL;
	}

	_record_set._res = mysql_store_result(_mysql_handle);
	if(NULL == _record_set._res)
	{
		print_error();
		return NULL;
	}

	return &_record_set;
}

//执行插入命令
bool DbConnect::insert_record(
					 const char* table,
					 const char* columns,
					 const char* values)
{
	if(NULL == table || NULL == columns || NULL == values)
		return 1;

	char sql[1024] = {0};
	sprintf(sql, "insert into %s(%s) values(%s)", table, columns, values);

	//printf("result:%s",sql);
	int ret = mysql_query(_mysql_handle, sql);
	if(0 != ret)
	{
		print_error();
		return -1;
	}

	return 0;
}

//执行更新命令
bool DbConnect::update_record(
					 const char* table,
					 const char* condition,
					 const char* fmt, ...)
{
	if(NULL == table || NULL == fmt)
		return -1;

	char buff[512] = {0};

	va_list valist;
	va_start(valist, fmt);
	int ret = vsnprintf(buff, sizeof(buff), fmt, valist);
	va_end(valist);

	char sql[1024] = {0};
	sprintf(sql, "update %s set %s where %s", table, buff, condition);

	ret = mysql_query(_mysql_handle, sql);
	if(0 != ret)
	{
		print_error();
		return -2;
	}

	return 0;
}

//执行查询命令
DbRecordset* DbConnect::exec_proc_record(
									  const char* procedure_name,
									  const char* fmt, ...)
{
	if(NULL == procedure_name)
		return NULL;

	_record_set.free_record();

	char buff[512] = {0};
	va_list valist;
	va_start(valist, fmt);
	int ret = vsnprintf(buff, sizeof(buff), fmt, valist);
	va_end(valist);

	char sql[1024] = {0};
	sprintf(sql, "call %s(%s);", procedure_name, buff);

	ret = mysql_query(_mysql_handle, sql);
	if(0 != ret)
	{
		print_error();
		return NULL;
	}

	_record_set._res = mysql_store_result(_mysql_handle);
	if(NULL == _record_set._res)
	{
		print_error();
		return NULL;
	}

	return &_record_set;
}

//执行查询命令
bool DbConnect::exec_proc(
	const char* procedure_name,
	const char* fmt, ...)
{
	if(NULL == procedure_name)
		return false;

	char buff[512] = {0};
	va_list valist;
	va_start(valist, fmt);
	int ret = vsnprintf(buff, sizeof(buff), fmt, valist);
	va_end(valist);

	char sql[1024] = {0};
	sprintf(sql, "call %s(%s);", procedure_name, buff);

	ret = mysql_query(_mysql_handle, sql);
	if(0 != ret)
	{
		print_error();
		return false;
	}

	return true;
}

