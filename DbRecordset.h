/*
 * db_mysql.h
 *
 *  Created on: 2015年9月17日
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
	//获取制定字段名称
	char* get_field_name(int field_num);

	//获取查询记录数目
	int get_record_num();

	//释放记录
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
