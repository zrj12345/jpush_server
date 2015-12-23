/*
 * DbPool.h
 *
 *  Created on: 2015Äê9ÔÂ17ÈÕ
 *      Author: root
 */

#ifndef DB_POOL_H_
#define DB_POOL_H_

#include <list>
#include <semaphore.h>
#include "DbConnect.h"
#include "DbRecordset.h"

class DbPool
{
public:
	DbPool();
	~DbPool();

public:
	bool create_pool(int pool_size);
	void destroy_pool();

	DbConnect* get_connect();
	void release_connect(DbConnect* conn);

private:
	std::list<DbConnect*> _conn_pool;
	pthread_mutex_t _lock;
	sem_t _sem;

};

extern DbPool cloudcar_pool;

#endif /* DB_POOL_H_ */
