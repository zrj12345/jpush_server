/*
 * DbConnect.cpp
 *
 *  Created on: 2015��9��17��
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

//��ʼ�����ݿ�����
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

	//�������ӳ�ʱʱ��.
	mysql_options(_mysql_handle, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&timeout);

	//���ò�ѯ���ݿ�(select)��ʱʱ��
	mysql_options(_mysql_handle, MYSQL_OPT_READ_TIMEOUT, (const char *)&timeout);

	//����д���ݿ�(update,delect,insert,replace��)�ĳ�ʱʱ�䡣
	mysql_options(_mysql_handle, MYSQL_OPT_WRITE_TIMEOUT, (const char *)&timeout);
	
	mysql_options(_mysql_handle,MYSQL_SET_CHARSET_NAME,"utf8");

	connect();
}

//�ر����ݿ�����
void DbConnect::close()
{
	if(NULL == _mysql_handle)
		return ;

	_record_set.free_record();
	mysql_close(_mysql_handle);
	//mysql_thread_end();//���ڶ��߳�
}

//��ȡ������
int DbConnect::get_error_no()
{
	if(NULL == _mysql_handle)
		return -1;

	return mysql_errno(_mysql_handle);
}

//��ȡ����ԭ��
const char* DbConnect::get_error_info()
{
	if(NULL == _mysql_handle)
		return "";

	return mysql_error(_mysql_handle);
}

//��ӡ������Ϣ
void DbConnect::print_error()
{
	if(NULL == _mysql_handle)
		return ;

	printf("errorno:%d, errorinfo:%s\n", get_error_no(), get_error_info());
}

//�������ݿ�
bool DbConnect::connect()
{
	if(NULL == _mysql_handle)
		return false;
	
	
	//mysql_thread_init();  //Ϊ���߳��̰߳�ȫʹ�õ�
	//CLIENT_MULTI_STATEMENTS:֪ͨ���������ͻ��˿����ڵ����ַ����ڷ��Ͷ�����䣨�ɡ�;�������������δ���øñ�־������ֹ�����ִ�С�
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

//������ݿ������Ƿ�����
bool DbConnect::ensure_connected()
{
	if(NULL == _mysql_handle)
		return false;

	if(0 != mysql_ping(_mysql_handle))
		return connect();

	return true;
}



//�����ύģʽ
void DbConnect::set_commit_mode()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_autocommit(_mysql_handle, 1);
}

//����ʼ
void DbConnect::transaction()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_query(_mysql_handle, "start transaction");
}

//�ύ����
void DbConnect::commit()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_commit(_mysql_handle);
}

//�ع�����
void DbConnect::rollback()
{
	if(NULL == _mysql_handle)
		return ;

	mysql_rollback(_mysql_handle);
}

//��������
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

//д������
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

//���������
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

//ִ�в�ѯ����
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

//ִ��mysql���������
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

//ִ�в�������
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

//ִ�и�������
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

//ִ�в�ѯ����
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

//ִ�в�ѯ����
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

