#ifndef  _PTHREAD_HANDLE_H_
#define  _PTHREAD_HANDLE_H_


#include "files.h"
#include "DbPool.h"
#include "redisclient.h"
#include <pthread.h>
#include <list>


typedef void  *(*event_deal)(void *arg);


class pthread_handle{
public:
	struct func_arg{
		event_deal func_pointer;
		void* arg;
	};
	~pthread_handle();
	pthread_handle(int thread_num = 4):max_thread_num(thread_num){};
	void  pool_event_init(int thread_num);
	int   thread_run();
	int   pool_destroy();
	int   pool_event_add(event_deal func,void *arg);
	static void* thread_event(void *arg);
	
private:
   pthread_cond_t    queue_ready; 
   pthread_mutex_t   queue_lock;
   list<func_arg>    event_list;
   void   	*event_arg;
   pthread_t 		*threadid;
   int 	max_thread_num;
   bool shutdown;
};


#endif