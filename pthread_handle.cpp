
#include "pthread_handle.h"


//static event_handle *event_pool = NULL;

//ent_handle* event_handle::event_pool = NULL;
//event_handle *event_pool;


int pthread_handle::thread_run()
{
	//cout<<"start"<<endl;
	while(1)
	{
		
		
		pthread_mutex_lock(&(queue_lock));
		
		while (event_list.empty() && !shutdown)
		{
		//	 printf ("thread 0x%x is waiting\n", pthread_self ());
			pthread_cond_wait (&(queue_ready), &(queue_lock));
		}
		
		if(shutdown)
		{
			pthread_mutex_unlock (&(queue_lock));
		//	printf ("thread 0x%x will exit\n", pthread_self ());
			 pthread_exit(NULL);
		}
		
		//printf ("thread 0x%x is starting to work\n", pthread_self ());
		

		func_arg p_event  = event_list.front();
		event_list.pop_front();
		
			//exec 
			(*(p_event.func_pointer))(p_event.arg);
		pthread_mutex_unlock(&(queue_lock));
			
		
	}
	pthread_exit(NULL);
}


void* pthread_handle::thread_event(void* arg)
{
  	pthread_handle *done=(pthread_handle*)arg;
  	done->thread_run();
}

void pthread_handle::pool_event_init(int thread_num)
{
	pthread_mutex_init(&(queue_lock), NULL);
	pthread_cond_init(&(queue_ready), NULL);

	max_thread_num = thread_num;
	shutdown = false;
	
	int size = max_thread_num * sizeof(pthread_t);
	threadid = new pthread_t[size];
	for(int i = 0;i<max_thread_num;i++)
	{
		pthread_create(&(threadid[i]),0,thread_event,this);
	}
}





int pthread_handle::pool_destroy()
{
  if (shutdown)
	  return -1;/*防止两次调用*/
   shutdown = true;
   
   
   pthread_cond_broadcast(&(queue_ready));
   
	
   for (int i = 0; i < max_thread_num; i++)
	   pthread_join (threadid[i], NULL);
   delete[] threadid;
   
    cout<<"wait"<<endl;
    pthread_mutex_destroy(&(queue_lock));
    pthread_cond_destroy(&(queue_ready));
   
   return 0;
}

pthread_handle::~pthread_handle()
{
	//delete threadid;
}



int pthread_handle::pool_event_add(event_deal func,void *arg)
{
	pthread_mutex_lock(&(queue_lock));
	
		func_arg argTemp;
		argTemp.func_pointer = func;
		argTemp.arg = arg;
	
		event_list.push_back(argTemp);
		
	//	cout<<"event_arg"<<*((int *)event_arg)<<endl;
		//cout<<"arg"<<*((int *)arg)<<endl;
	//	cout<<"size"<<event_list.size()<<endl;
		//event_list.back()
	pthread_mutex_unlock(&(queue_lock));
	pthread_cond_signal(&(queue_ready));
	
	return 0;
}














