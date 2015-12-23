#include "event_drive.h"

#include <sys/types.h>
#include <sys/stat.h>
bool event_handle::event_init(string init_arg)
{	
	//守护进程将工作目录设置为"/",后面读取文件的时候要指定绝对路径,不然程序读不到数据,会出错的。
	

	jpush_arg arg;
	arg.read_config(init_arg.c_str());
	gain_config_value(arg);
		
	jpush_arg::runtime_parameter::db_connect_parameter    		db_arg = argEvent.get_runtime_value().db_par_;
	jpush_arg::runtime_parameter::redis_connect_parameter 		redis_arg = argEvent.get_runtime_value().redis_par_;
	jpush_arg::runtime_parameter::logger_parameter				log_arg = argEvent.get_runtime_value().log_par_;
	
	string enable_daemon = argEvent.get_runtime_value().enable_daemon;

	if(enable_daemon == "yes")  
	{
		daemon(1,0);
	}
	//初始化系统日志
	if(log_arg.enable_syslog == "yes")
	{
		openlog(log_arg.log_name.c_str(),LOG_PID | LOG_CONS, LOG_LOCAL0);
		string log_info = "start syslog";
		event_logger(LOG_INFO,log_info.c_str());
	}

	//初始化文本日志
	if(log_arg.file_log_path != "")
	{
		jpush_arg::log_fptr = fopen(log_arg.file_log_path.c_str(),"a+");
		string log_info = "start file logger";
		event_logger(LOG_INFO,log_info.c_str());
	}

	if(log_arg.enable_console == "yes")
	{
		string log_info = "start console debug";
		event_logger(LOG_INFO,log_info.c_str());
	}


	//初始化mysql数据库
	if(db_arg.ip == "" || db_arg.port == 0 || db_arg.dbname == "" || db_arg.password == "" || db_arg.loginname == "")
	{
		//参数不全,使用默认的链接
		event_logger(LOG_ERR,"mysql connection parameter--ip:%s,port:%d,dbname:%s,loginname:%s,password:%s err \n   use default parameter:\nip:localhost,port:3306,dbname:mysql,loginname:root,password:000000",\
			db_arg.ip.c_str(),db_arg.port,db_arg.dbname.c_str(),db_arg.loginname.c_str(),db_arg.password.c_str());
		db_event.open("localhost",3306,"mysql",5,"root","000000");
	}
	else
	{
		db_event.open(db_arg.ip.c_str(),db_arg.port,db_arg.dbname.c_str(),db_arg.time_out,db_arg.loginname.c_str(),db_arg.password.c_str());
	}

	//初始化redis
	if(redis_arg.redis_host == "" || redis_arg.redis_port == 0)
	{
			event_logger(LOG_ERR,"redis connection parameter err,use default parameter\n  ip:127.0.0.1,port:6379");
			redis_connect("127.0.0.1",6379,0);
	}
	else
	{
		redis_connect(redis_arg.redis_host,redis_arg.redis_port,redis_arg.dbindex);			
	}
	//初始化线程池
	if(argEvent.get_runtime_value().thread_num == 0)
	{
		pthread_event.pool_event_init(1);	
	}
	else 
	{
		pthread_event.pool_event_init(argEvent.get_runtime_value().thread_num);	
	}
}


event_handle::event_handle(string init_arg)
{
	event_init(init_arg);
}

event_handle::event_handle()
{
	db_event.open("localhost",3306,"mysql",5,"root","000000");
	pthread_event.pool_event_init(20);
	redis_instance.open("127.0.0.1",6379,0);
	openlog("Jpush_Event",LOG_PID | LOG_CONS, LOG_LOCAL0);
	event_logger(LOG_INFO,"start logging\n");
	
	string log_info = "start logging at time:" + get_current_time();
	
	event_logger(LOG_INFO,log_info.c_str());
	
}
void event_handle::event_logger(int level,const char* fmt,...)
{

	va_list		arg_ptr;
	va_list		arg_cpy;
	va_list		arg_log;
	va_start(arg_ptr,fmt);
	va_copy(arg_cpy,arg_ptr);
	va_copy(arg_log,arg_ptr);

	string cur_time = get_current_time();
	jpush_arg::runtime_parameter::logger_parameter log_arg = argEvent.get_runtime_value().log_par_;

	//写系统日志
	//if(log_arg.enable_syslog == "yes")
	{
		syslog(level,"%s:",cur_time.c_str());
		vsyslog(level,fmt,arg_ptr);
	}
	//写文本日志
	if(jpush_arg::log_fptr != NULL)
	{
		//fwrite(jpush_arg::log_fptr,"%s    :",cur_time.c_str());
		fwrite(cur_time.c_str(),cur_time.size(),1,jpush_arg::log_fptr);
		fwrite(" --- ",strlen(" --- "),1,jpush_arg::log_fptr);
		vfprintf(jpush_arg::log_fptr,fmt,arg_log);
		fprintf(jpush_arg::log_fptr,"\n");
		fflush(jpush_arg::log_fptr);
	}
	//写控制台
	if(log_arg.enable_console == "yes")
	{
		fprintf(stdout,"%s:",cur_time.c_str());
		vfprintf(stdout,fmt,arg_cpy);
		fprintf(stdout,"\n");
	}

	va_end(arg_ptr);
	va_end(arg_log);
	va_end(arg_cpy);	
}

event_handle::~event_handle()
{
	releae_resources();
}

void event_handle::redis_connect(const string& redis_ip,int redis_port,int dbindex)
{
	while(1)
	{
		try{
			redis_instance.open(redis_ip,redis_port,dbindex);
			return ;
		}
		catch (redis::redis_error & e) 
		{
			event_logger(LOG_ERR,"got exception:%s,please check redis-server status",e.what());
			usleep(10);
			continue;
		}
	}
}


 
bool event_handle::redis_safe_check_exists(string& temp)
{
	while(1)
	{
		try{
			if(redis_instance.exists(temp) == false) 
			{
				return false;
			}
			else 
				return true;
		}catch(redis::redis_error& e) {
			event_logger(LOG_ERR,"got exception:%s,redis primary key err,please check",e.what());
			usleep(10);
			continue;
		}
	}
			
}

static void bytes2Hex(char *hexString, void *bytes, int len)
{
    int i;
    unsigned char *rawBytes;
    char tmpStr[3];
    
    rawBytes = (unsigned char *)bytes;
    for (i = 0, hexString[0] = '\0';i < len;i++)
    {
        sprintf(tmpStr, "%02x", rawBytes[i]);
        strcat(hexString, tmpStr);
    }
}

bool event_handle::get_data_redis()
{
	jpush_arg arg;
	string temp,ltemp,content,title;
	vector<string> string_value,string_key;

	//string_key.push_back("RegistrationID");
	string_key.push_back("receiver_value");
	string_key.push_back("platform"); 
	string_key.push_back("customizedID");
	string_key.push_back("receiver_type");
	string_key.push_back("n_title");
	string_key.push_back("n_content");
	string_key.push_back("n_extras1:key");
	string_key.push_back("n_extras1:value");

	temp = argEvent.get_runtime_value().redis_primary_key;
	
	if(redis_safe_check_exists(temp) == false) 
	{
		return false;
	}
	ltemp = redis_instance.lpop(temp); 
	

	cout<<"ltemp:"<<ltemp<<endl;
	char strTemp[1024] = {0};
	memcpy(strTemp,ltemp.c_str(),ltemp.size());
	event_logger(LOG_INFO,"get data from redis:%s",strTemp);
	arg.string_filter(strTemp,",",string_value);
	memset(strTemp,0,sizeof strTemp);
	if(string_value.size() <= 0)
	{
		event_logger(LOG_ERR,"data from redis not enough");
		return false;
	}

	temp = string_value.at(1); 
	if(temp != "0" && temp != "1")
	{
		event_logger(LOG_ERR,"send type:%s which can only be 0 or 1...",temp.c_str());
		return false;
	}
	
	switch(strtol(temp.c_str(),NULL,10))
	{
		case 0:
			string_value.at(1) = "android";
			break;
		case 1:
			string_value.at(1) = "ios";
		default:
			event_logger(LOG_ERR,"send type:%s undefined,exit...",temp.c_str());
			return false;
	}
	
	temp = string_value.at(3);
	if(temp != "2" && temp != "3" && temp != "4" && temp != "5")
	{
		event_logger(LOG_ERR,"receiver_type:%s which value can only be 2,3,4,5...",temp.c_str());
		return false;
	}

	//get warnning type 
	temp = string_value.at(4);
	string_value.push_back("warnning_type");
	switch(strtol(temp.c_str(),NULL,10))
	{
		case 0:
			//车辆故障
			title = "车辆故障";
			content = "";
			string_value.push_back("0");
			break;
		case 1:
			//车辆超速
			title = "您的爱车，超速行驶";
			content = "当前速度:" + string_value.at(5);
			string_value.push_back("7");
			break;
		case 3:
			//设备故障
			title = "设备故障";   //---GPS故障,没有内容
			//content = "非法移动,超过" + string_value.at(5) + "米";
			content = "";
			string_value.push_back("2");
			break;
		case 4:
			//电瓶电压
			title = "您的爱车,电压过低";
			content = "当前电压:" + string_value.at(5);
			string_value.push_back("6");
			break;
		case 6:
			//ECM故障 
			title = "ECM故障";
			content = "车辆故障码:" + string_value.at(5);
			string_value.push_back("4");
			break;
		case 7:
			//冷却液温度过高 
			title = "您的爱车，冷却液温度过高";
			content = "冷却液温度:" + string_value.at(5);
			string_value.push_back("5");
			break;
		case 8:
			//冷却液温度过低 
			title = "您的爱车，冷却液温度过低";
			content = "冷却液温度:" + string_value.at(5);
			string_value.push_back("5");
			break;
		case 9:
			//车辆保养 
			title = "您的爱车，需要保养";
			//content = "车辆故障码" + string_value.at(4);
			content = "";
			string_value.push_back("3");
			break;
		case 11:
			//车辆碰撞 
			title = "您的爱车，发生了碰撞";
			//content = "车辆故障码" + string_value.at(4);
			content = "";
			string_value.push_back("8");
			break;
		case 50:
			//车辆安防
			title = "您的爱车，非法移动";
			content = "非法位移，超过:" + string_value.at(5) + "米";
			string_value.push_back("1");
			break;
		case 100:
			//自定义消息,没有附加信息
			title = "您有最新消息";
			content = string_value.at(5);
		default:
			event_logger(LOG_ERR,"current warnnig type:%s undefined,exit...",temp.c_str());
			return false;
	}

	//修改从redis获取的数据,修改第3项,和第4项。
	string_value.at(4) = title;
	string_value.at(5) = content;
		
	if(string_value.size() != string_key.size()) 
	{
		event_logger(LOG_ERR,"data count from redis is not enough\n");
		usleep(5);
		return false;
	}
	jpush_arg::fifo_jpush_data p_cmd = {0};
	for(int i = 0;i<string_key.size();i++)
	{
		argEvent.config_value(string_value.at(i).c_str(),string_key.at(i).c_str(),p_cmd);
	}

	jpush_arg().consist_cmd_data(p_cmd);

	if(argEvent.is_data_ok(p_cmd))
	{
		argEvent.store_cmd_data(p_cmd);
		return true;
	}
	else
	{
		event_logger(LOG_ERR,"push_data err");
		return false;
	}
}




static inline void consist_push_data(jpush_arg::fifo_jpush_data& argTemp,string& pForm)
{
		ostringstream pFormTemp;		
		MD5_CTX ctx;
		unsigned char md5Value[16] = {0};
		char md5String[33] = {0};
    
		//make verification_code---->md5String store result 
		pFormTemp<<argTemp.sendno<<argTemp.receiver_type<<argTemp.receiver_value<<argTemp.master_secret;
		string pTemp = pFormTemp.str();
        MD5_Init(&ctx);
        MD5_Update(&ctx, pTemp.c_str(), strlen(pTemp.c_str()));
        MD5_Final(md5Value, &ctx);
        bytes2Hex(md5String, md5Value, 16);
        
		//clear data used by pFormTemp
		pFormTemp.str("");
		
		
		//make msg_content		
		jpush_arg  jpushTemp;
		string str_msg_content = jpushTemp.serialization_to_json(argTemp.msg_content);
	

		//consist all data push need
 		pFormTemp<<"sendno="<<argTemp.sendno  		\
				<<"&app_key="<<argTemp.app_key;
		if(argTemp.receiver_type >=2 && argTemp.receiver_type <=5)
		{
				pFormTemp<<"&receiver_type="<<argTemp.receiver_type;
		}
		if(argTemp.receiver_value != "")
		{
			pFormTemp<<"&receiver_value="<<argTemp.receiver_value;
		}
				pFormTemp<<"&verification_code="<<(strcasecmp(md5String,argTemp.verification_code.c_str())?md5String:argTemp.verification_code)	\
				<<"&msg_type="<<argTemp.msg_type	\
				<<"&msg_content="<<str_msg_content  \
				<<"&send_description="<<argTemp.send_description	\
				<<"&platform="<<argTemp.platform;
		if(argTemp.time_to_live >= 0)
		{
				pFormTemp<<"&time_to_live="<<argTemp.time_to_live;
		}
		pForm = pFormTemp.str();

		//make apns_production
		pFormTemp.str("");
		if(argTemp.apns_production == 0 || argTemp.apns_production == 1)
		{
					pFormTemp<<"&apns_production="<<argTemp.apns_production;
					pForm += pFormTemp.str();
		}
		
		//make override_msg_id
		pFormTemp.str("");
		if(argTemp.override_msg_id != "")
		{
					pFormTemp<<"&override_msg_id="<<argTemp.override_msg_id;
					pForm += pFormTemp.str();
		}
		pFormTemp.str("");
		//cout<<"pForm = "<<pForm<<endl;
}


static inline void consist_mysql_data(jpush_arg::fifo_jpush_data& argTemp,string& store_value)
{
	ostringstream consistable;

	//read from file sendno,app_key
	//read form redis-server customizedID(eg owner_id,ect..) platform(app_type),receiver_type(push_type),receiver_value(push_id),msg_content.n_content(msg_title),msg_content.n_title(msg_title)
	//time_to_live(msg_live),msg_type(msg_show_type)
   consistable<<"'"<<argTemp.customizedID<<"',"<<argTemp.sendno<<",'"<<argTemp.app_key<<"','"<<argTemp.platform<<"',"<<argTemp.receiver_type \
		  <<",'"<<argTemp.receiver_value<<"','"<<argTemp.msg_content.n_content<<"','"<<argTemp.msg_content.n_title;


	consistable<<"',";
	/*
	if(argTemp.time_to_live != -1)	
	{
		consistable<<argTemp.time_to_live;
	}
	else
	{
		consistable<<"''";
	}
	*/
	consistable<<argTemp.time_to_live;
	consistable<<","<<argTemp.msg_type<<"";

		 store_value = consistable.str();
}
string event_handle::get_current_time()
{
  time_t t;
  t = time(0);
  struct tm *tt = NULL;
  tt = localtime(&t);
  char time_format[25] = {0};
  sprintf(time_format,"%04d-%02d-%02d %02d:%02d:%02d",\
                  tt->tm_year+1900,tt->tm_mon+1,tt->tm_mday,\
                  tt->tm_hour,tt->tm_min,tt->tm_sec);
				  
	cur_time = time_format;
	return cur_time;
}
bool event_handle::data_push(jpush_arg::fifo_jpush_data& argTemp)
{
    CURL *curl;
    CURLcode res;
	string pForm;

	string logger_record;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl)
	{
		char url[128] = {0};
		
        curl_easy_setopt(curl, CURLOPT_URL,jpush_arg::jpush_url.c_str());        
       // curl_easy_setopt(curl, CURLOPT_URL, "https://api.jpush.cn:443/sendmsg/v2/sendmsg");	
		consist_push_data(argTemp,pForm);
		//cout<<"pForm"<<pForm<<endl;

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pForm.c_str());       
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_POST,1); //set Content-Type:application/x-www-form-urlencoded
		
		int fail_try = 3;
        do
		{
			res = curl_easy_perform(curl);
		}while((res != CURLE_OK) && fail_try--);
		
			if(res != CURLE_OK)
				event_logger(LOG_ERR,"curl_easy_perform() failed: %s",curl_easy_strerror(res));
				//fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
			else
			{
				ostringstream streamTemp;
				streamTemp<<"sendno:"<<argTemp.sendno<<",with app_key:"<<argTemp.app_key<<" push successfully!";
				logger_record = streamTemp.str();
				event_logger(LOG_INFO,logger_record.c_str());
			}
		
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
	
    curl_global_cleanup();
    
    return true;
}

bool event_handle::store_data_msql(jpush_arg::fifo_jpush_data& argTemp)
{
		string store_value;
	
		string table_name = argEvent.get_runtime_value().db_par_.db_table_name;

		consist_mysql_data(argTemp,store_value);
		event_logger(LOG_INFO,("data being stored mysql:" + store_value).c_str());

		db_event.insert_record(table_name.c_str(),"owner_id,push_no,app_key,app_type,push_type,push_id,msg_content,msg_title,msg_live,msg_show_type"\
						,store_value.c_str());
}
int event_handle::background()
{
	return daemon(0,0);

}
int event_handle::daemon_init()
{
	int cpid;

	signal(SIGALRM, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCLD, SIG_IGN); 
	signal(SIGPIPE, SIG_IGN);

	if((cpid = fork()) < 0) 
	{ 
		perror("create daemon program error\n");
		exit(-1);
	}
	else if(cpid>0)  
	{
		exit(0);               
	}

	if(setsid() == -1) 
	{
		perror("setsid error\n");
		exit(-1);
	}

	signal(SIGHUP,SIG_IGN);
	if((cpid=fork() ) < 0)
	{   
		perror("create daemon program error\n");
		exit(-1);
	}
	else if(cpid > 0) 
	{
		exit(0); 
	}

	return 0;
}

 
void* event_handle::func_routine(void *arg)
{
	event_handle *dosome = (event_handle *)arg;
	jpush_arg::fifo_jpush_data argTemp;
	while(1)
	{
		do{
			dosome->argEvent.get_value(argTemp);
		}while(!jpush_arg().is_data_ok(argTemp));
		
		dosome->data_push(argTemp);
		dosome->store_data_msql(argTemp);
	}
}
void event_handle::event_run()
{
	pthread_event.pool_event_add(func_routine,this);
	
	while(1)
	{
		if(get_data_redis())
		{
			event_logger(LOG_INFO,"data_ok,thread working now");
			//usleep(5);
		}	
		else 
			sleep(5);
	}
	releae_resources();
}

	
void event_handle::gain_config_value(jpush_arg push_arg)
{
	//argEvent.config_value(push_arg.get_value().app_key.c_str(),"app_key");
	argEvent = push_arg;
}

bool event_handle::store_data_redis()
{		
	jpush_arg::fifo_jpush_data argTemp;
	argEvent.get_value(argTemp);
	string storename = argTemp.record_name;
	ostringstream  hvalue;
	hvalue<<argTemp.sendno;
	redis_instance.hset(storename,"sendno",hvalue.str());
		hvalue.str("");
	redis_instance.hset(storename,"app_key",argTemp.app_key);
		hvalue<<argTemp.receiver_type;
	redis_instance.hset(storename,"receiver_type",hvalue.str());
		hvalue.str("");
	redis_instance.hset(storename,"receiver_value",argTemp.receiver_value);
	redis_instance.hset(storename,"verification_code",argTemp.verification_code);
		hvalue<<argTemp.msg_type;
	redis_instance.hset(storename,"msg_type",hvalue.str());
		hvalue.str("");
	redis_instance.hset(storename,"n_content",argTemp.msg_content.n_content);
		hvalue<<argTemp.msg_content.n_builder_id;
	redis_instance.hset(storename,"n_builder_id",hvalue.str());
		hvalue.str("");
	redis_instance.hset(storename,"n_title",argTemp.msg_content.n_title);
	redis_instance.hset(storename,"send_description",argTemp.send_description);
	redis_instance.hset(storename,"platform",argTemp.platform);
		hvalue<<argTemp.apns_production;
	redis_instance.hset(storename,"apns_production",hvalue.str());
		hvalue.str("");
		hvalue<<argTemp.time_to_live;
	redis_instance.hset(storename,"time_to_live",hvalue.str());
		hvalue.str("");
		hvalue<<argTemp.override_msg_id;
	redis_instance.hset(storename,"override_msg_id",hvalue.str());
		hvalue.str("");
		hvalue<<argTemp.master_secret;
	redis_instance.hset(storename,"master_secret",hvalue.str());
		hvalue.str("");
}
	


void event_handle::releae_resources()
{
	redis_instance.shutdown();
	db_event.close();
	pthread_event.pool_destroy();
}












