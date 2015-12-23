/*
 * db_mysql.h
 *
 *  Created on: 2015��9��17��
 *      Author: root
 */

#ifndef DB_RECORDSET_H_
#define DB_RECORDSET_H_

extern "C" {
#include "mysql/mysql.h"
}

class DbRecordset
{
public:
	DbRecordset();

	~DbRecordset();

public:
	//��ȡ�ƶ��ֶ�����
	char* get_field_name(int field_num);

	//��ȡ��ѯ��¼��Ŀ
	int get_record_num();

	//�ͷż�¼
	void free_record();

	void movefirst();

	MYSQL_ROW movenext();


public:
	MYSQL_RES* _res;

private:
	int _record_count;
	int _first_record;

};


#endif /* DB_RECORDSET_H_ */
