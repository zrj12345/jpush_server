#ifndef _FILES_H_
#define _FILES_H_

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <list>
#include <semaphore.h>

#include <iconv.h>
extern "C"{
	#include "parson.h"
}
using namespace std;


#define READ_CONFIG_BUF_SIZE 256
#define DATA_LENGTH  12

class jpush_arg
{
 public:
	jpush_arg();
	jpush_arg(string file);
	struct fifo_jpush_data{
			struct extras_data{
				string key;
				string value;
			};
		

		typedef struct tagmsg_content{
				int 		 n_builder_id;
				string  	 n_title;
				string  	 n_content;
				vector<extras_data>  n_extras;
				}msg;
	
		int 	sendno;
		string  app_key;
		int 	receiver_type;
		string  receiver_value;
		string  verification_code;
		int 	msg_type;
		msg		msg_content;
		string  send_description;
		string  platform;
		int     apns_production;
		int     time_to_live;
		string  override_msg_id;
		string  record_name;
		string  master_secret;
		string  RegistrationID;
		string  customizedID;
	};

	struct runtime_parameter{
		struct db_connect_parameter{
			string 		ip;
			int 		port;
			string 		dbname;
			string 		loginname;
			string 		password;
			int 		time_out;	
			string	    db_table_name;
		};
		struct redis_connect_parameter{
			string		redis_host;
			int 		redis_port;
			int			dbindex;
		};
		
		struct logger_parameter{
			string 			log_level;
			string			log_name;
			string			enable_syslog;
			string			file_log_path;
			string			enable_console;

		};
		
		db_connect_parameter     db_par_;
		redis_connect_parameter  redis_par_;
		logger_parameter  		 log_par_;
		int 					 thread_num;
		string					 redis_primary_key;
		string					 enable_daemon;
	};
	void config_value(const char *value_src,const char *keyword,fifo_jpush_data& p_cmd);
	void get_value(jpush_arg::fifo_jpush_data& temp);
	jpush_arg::runtime_parameter& get_runtime_value();
	int  is_data_ok(fifo_jpush_data& p_cmd);
	int  read_config(const char *file);
	void code_convert(char *inbuf,char *outbuf,const char *from_charset,const char *to_charset);
	string& serialization_to_json(fifo_jpush_data::msg &argmsgTemp);
	void print_value(fifo_jpush_data& p_cmd);
	bool reset_value();
	static string jpush_url;
	static FILE* log_fptr;
	void string_filter(char *src,const char* delim,vector<string>& res);
	void store_cmd_data(fifo_jpush_data& p_cmd);
	void init_data(jpush_arg::fifo_jpush_data& arg);
	void consist_cmd_data(fifo_jpush_data& p_cmd);
	void del_cmd_head();
	bool check_cmd_empty();
	void clear_resources();
 private:
	  list<fifo_jpush_data> cmd_head;
	  sem_t g_sem_full;
	  sem_t g_sem_empty;
	  pthread_mutex_t g_mutex;
	  runtime_parameter run_arg;
	  string file_path;
	  string json_data;
	  static fifo_jpush_data p_file;
	  static const char keywords[12][20];
};




const char keywords[12][20] = {"sendno","app_key","receiver_type","receiver_value","verification_code","msg_type","msg_content","send_description","platform","apns_production","time_to_live","override_msg_id"};
#endif









