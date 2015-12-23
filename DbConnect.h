/*
 * db_mysql.h
 *
 *  Created on: 2015��9��17��
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

	//�ر����ݿ�����
	void close();

	//��ȡ������
	int get_error_no();

	//��ȡ����ԭ��
	const char* get_error_info();

	//��ӡ������Ϣ
	void print_error();

	//�������ݿ�
	bool connect();

	//������ݿ������Ƿ�����
	bool ensure_connected();

	//�����ύģʽ
	void set_commit_mode();

	//����ʼ
	void transaction();

	//�ύ����
	void commit();

	//�ع�����
	void rollback();

	//��������
	bool lock_table_read(const char* psTableName);

	//д������
	bool lock_table_write(const char* psTableName);

	//���������
	bool unlock_table(const char* psTableName);

	
	//ִ�в�ѯ����
	DbRecordset* query_record(
						const char* table,
						const char* columns,
						const char* condition,
						const char* sort,
						const char* limit);
					
	DbRecordset*  do_mysql_cmd(const char* cmd);

	//ִ�в�������
	bool insert_record(
						 const char* table,
						 const char* columns,
						 const char* values);

	//ִ�и�������
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
