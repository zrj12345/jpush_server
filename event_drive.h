#ifndef  _EVENT_DRIVE_H_
#define  _EVENT_DRIVE_H_

#include <openssl/md5.h>
#include <curl/curl.h>
#include <syslog.h>
#include <signal.h>
#include <sys/param.h>
#include <stdarg.h>

#include "files.h"
#include "DbPool.h"
#include "redisclient.h"
#include "pthread_handle.h"

class event_handle{
public:
	bool get_data_redis();
	bool store_data_redis();
	bool event_init(string init_arg);
	bool data_push(jpush_arg::fifo_jpush_data& argTemp);
	bool store_data_msql(jpush_arg::fifo_jpush_data& argTemp);
	void event_run();
	int  background();
	void log_event();
	void gain_config_value(jpush_arg push_arg);
	void releae_resources();
	void event_logger(int level,const char* fmt,...);
	bool redis_safe_check_exists(string& temp);
	void redis_connect(const string& redis_ip,int redis_port,int dbindex);
	string get_current_time();
	int daemon_init();
	event_handle(string init_arg);
	event_handle();
	~event_handle();
private:
	string 		  cur_time;
	DbPool        _dbpool;
	jpush_arg     argEvent;
	DbConnect     db_event;
	redis::client redis_instance;
	pthread_handle pthread_event;
	static void* func_routine(void *arg);
};




#endif