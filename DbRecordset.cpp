/*
 * DbRecordset.cpp
 *
 *  Created on: 2015年9月17日
 *      Author: root
 */



#include "DbRecordset.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

DbRecordset::DbRecordset() : _res(NULL)
{
}

DbRecordset::~DbRecordset()
{
}

//获取查询记录数目
int DbRecordset::get_record_num()
{
	if(NULL == _res)
		return -1;

	//mysql_num_rows返回结果集中的行数
	return mysql_num_rows(_res);
}

//释放记录
void DbRecordset::free_record()
{
	if(NULL == _res)
		return ;

	mysql_free_result(_res);
	_res = NULL;
}

void DbRecordset::movefirst()
{
	if(NULL == _res)
		return;

	_first_record = 0;
	_record_count = get_record_num();
}

MYSQL_ROW DbRecordset::movenext()
{
	if(NULL == _res || _first_record >= _record_count)
		return NULL;

	MYSQL_ROW row = mysql_fetch_row(_res);
	_first_record++;

	return row;
}

