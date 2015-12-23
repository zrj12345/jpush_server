/*
 * DbPool.cpp
 *
 *  Created on: 2015Äê9ÔÂ17ÈÕ
 *      Author: root
 */

#include "DbPool.h"
#include <pthread.h>

DbPool cloudcar_pool;

void open(const char* ip, int port,const char* db, int timeout,const char* usrname, const char* password);
DbPool::DbPool()
{
	sem_init(&_sem,1 ,0);
	pthread_mutex_init(&_lock, NULL);
}

DbPool::~DbPool()
{
	sem_destroy(&_sem);
	pthread_mutex_destroy(&_lock);
}

bool DbPool::create_pool(int pool_size)
{
	if(_conn_pool.size() > 0)
		return true;

	for(int i = 0; i < pool_size; i++)
	{
		DbConnect* conn = new DbConnect;
		
		conn->open("127.0.0.1", 3306,
				"stu", 30,
				"root", "000000");


		_conn_pool.push_back(conn);
		sem_post(&_sem);
	}

	return true;
}

void DbPool::destroy_pool()
{
	std::list<DbConnect*>::iterator itor = _conn_pool.begin();

	for( ; itor != _conn_pool.end(); itor++)
	{
		DbConnect* conn = *itor;
		conn->close();
		delete conn;
	}

	_conn_pool.clear();
}



DbConnect* DbPool::get_connect()
{
	if(0 != sem_wait(&_sem))
		return NULL;

	pthread_mutex_lock(&_lock);

	DbConnect* conn = _conn_pool.front();
	_conn_pool.pop_front();

	pthread_mutex_unlock(&_lock);

	conn->ensure_connected();

	return conn;
}

void DbPool::release_connect(DbConnect* conn)
{
	pthread_mutex_lock(&_lock);
	_conn_pool.push_back(conn);
	pthread_mutex_unlock(&_lock);

	sem_post(&_sem);
}

