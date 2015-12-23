#include "files.h"


string jpush_arg::jpush_url = "https://api.jpush.cn:443/sendmsg/v2/sendmsg";
FILE*  jpush_arg::log_fptr = NULL;
jpush_arg::fifo_jpush_data jpush_arg::p_file;

#define NUM_MAX    1280
jpush_arg::jpush_arg()
{
	/*
	p_cmd.apns_production = -1;
	p_cmd.time_to_live = -1;
	p_cmd.sendno = -1;
	p_cmd.receiver_type = -1;
	*/
	sem_init(&g_sem_full,0,0);
	sem_init(&g_sem_empty,0,NUM_MAX);
	pthread_mutex_init(&g_mutex,NULL);
}
jpush_arg::jpush_arg(string file):file_path(file)
{
	/*
	p_cmd.apns_production = -1;
	p_cmd.time_to_live = -1;
	p_cmd.sendno = -1;
	p_cmd.receiver_type = -1;
	*/
	sem_init(&g_sem_full,0,0);
	sem_init(&g_sem_empty,0,NUM_MAX);
	pthread_mutex_init(&g_mutex,NULL);
}
void jpush_arg::clear_resources()
{
	sem_destroy(&g_sem_full);
	sem_destroy(&g_sem_empty);
	
	pthread_mutex_destroy(&g_mutex);
}
void jpush_arg::init_data(jpush_arg::fifo_jpush_data& p_cmd)
{	
	p_cmd.apns_production = -1;
	p_cmd.time_to_live = -1;
	p_cmd.sendno = -1;
	p_cmd.receiver_type = -1;
}
void jpush_arg::code_convert(char *inbuf,char *outbuf,const char *from_charset,const char *to_charset)
{
	iconv_t cd;
	cd = iconv_open(to_charset,from_charset);
	
	char **pin = &inbuf;
	char **pout = &outbuf;
	
	size_t inlen  = strlen(inbuf);
	size_t outlen = READ_CONFIG_BUF_SIZE;
	memset(outbuf,0,outlen);
	iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
	
	iconv_close(cd);
	
	return ;
}

/*
bool jpush_arg::string_filter(char *src,const char* delim,vector<string>& res)
{
        if(src == NULL || delim == NULL)  return false;
		string first;
		string temp;
		char *temp1 = (char *)malloc(strlen(src));
		memcpy(temp1,src,strlen(src));
		char *p = NULL;
		first = strtok(src,delim);
		if(strstr(src,delim) == src)
		{
			res.push_back("");
		}
		res.push_back(first);

        while(p = strtok(NULL,delim))
        {
                temp = p;
                res.push_back(temp);
        }
		char *ptr = temp1 + strlen(temp1) - strlen(delim);
		if(strstr(ptr,delim) == ptr)
		res.push_back("");
		free(temp1);
        return true;
}
*/


void jpush_arg::string_filter(char *src,const char* delim,vector<string>& res)
{
	char *front,*back;
	char temp[256] = {0};
	string stemp;
	
	
	front = src;
	
	while(front != src + strlen(src) -1)
	{
		if((back = strstr(front,delim)) == NULL)
		{
			memcpy(temp,front,(src + strlen(src) - front));
			stemp = temp;
			res.push_back(stemp);
			memset(temp,0,sizeof temp);
			break;
		}
		if((back = strstr(front,delim)) == src)
		{
			res.push_back("");
			front += strlen(delim);
		}
		else
		{
			memcpy(temp,front,(back - front));
			stemp = temp;
			res.push_back(stemp);
			memset(temp,0,sizeof temp);
			front = back + strlen(delim);
			if(*front == '\0')
			{
				res.push_back("");
				break;
			}
		}
	}
}
string& jpush_arg::serialization_to_json(fifo_jpush_data::msg &argmsgTemp)
{
	fifo_jpush_data::msg msgTemp = argmsgTemp;
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "n_content", msgTemp.n_content.c_str());
	
	if(msgTemp.n_content !="") 
		json_object_set_string(root_object, "n_title", msgTemp.n_title.c_str());
	
	if(msgTemp.n_builder_id != 0)
		json_object_set_number(root_object, "n_builder_id", msgTemp.n_builder_id);
	
	
	vector<fifo_jpush_data::extras_data> extrasTemp = msgTemp.n_extras;
	string n_extras_key,n_extras_value;
	
	vector<fifo_jpush_data::extras_data>::iterator it_extra_begin = extrasTemp.begin();
	for(it_extra_begin;it_extra_begin!=extrasTemp.end();it_extra_begin++)
	{	
		n_extras_key   = "n_extras." + (*it_extra_begin).key;
		n_extras_value = (*it_extra_begin).value;
		json_object_dotset_string(root_object, n_extras_key.c_str(), n_extras_value.c_str());
		n_extras_key   = "";
		n_extras_value = "";
	}
			//  json_object_dotset_value(root_object, "contact.emails", json_parse_string("[\"email@example.com\",\"email2@example.com\"]"));
	
	//方案1
	/*
    serialized_string = json_serialize_to_string_pretty(root_value);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
	*/
	
	//方案2
	//json_value_free(root_value);
	//return json_serialize_to_string_pretty(root_value);
	
	
	//方案3
	json_data = json_serialize_to_string_pretty(root_value);
	json_free_serialized_string(serialized_string);
    json_value_free(root_value);
	return json_data;
}
int jpush_arg::read_config(const char *file)
{
	file_path = file;
	FILE *in;
	char buffer[READ_CONFIG_BUF_SIZE], *token, *line;
	char buffer_convert[READ_CONFIG_BUF_SIZE] = {0};
	int i, lm = 0;
	fifo_jpush_data p_temp;
	init_data(p_temp);

	if (!(in = fopen(file_path.c_str(), "r"))) {
		printf("unable to open config file: %s", file_path.c_str());
		return 0;
	}

	while (fgets(buffer, READ_CONFIG_BUF_SIZE, in)) {
		code_convert(buffer,buffer_convert,"gb2312","utf-8");
		lm++;
		if (strchr(buffer_convert, '\n')) *(strchr(buffer_convert, '\n')) = '\0';

		if (strchr(buffer_convert, '#')) *(strchr(buffer_convert, '#')) = '\0';
		

		if (!(token = strtok(buffer_convert, " \t="))) continue;
		if (!(line = strtok(NULL, ""))) continue;

		/* eat leading whitespace */
		line = line + strspn(line, " \t=");
		/* eat trailing whitespace */
		for (i = strlen(line); i > 0 && isspace(line[i - 1]); i--);
		line[i] = '\0';
		
			//printf("token:%s--line:%s\n",token,line);
			config_value(line,token,p_temp);

		memset(buffer,0,sizeof buffer);
		memset(buffer_convert,0,sizeof buffer_convert);
	}
	p_file = p_temp;
	/*
	cout<<"******************************************************************************************"<<endl;
	print_value(p_file);
	cout<<"*****************************************************************************************"<<endl;
	*/
	fclose(in);
	return 1;
}	

void jpush_arg::consist_cmd_data(fifo_jpush_data& p_cmd)
{
	p_cmd.sendno		   = p_file.sendno;
	p_cmd.app_key		   = p_file.app_key;
	p_cmd.master_secret    = p_file.master_secret;
	p_cmd.time_to_live	   = p_file.time_to_live;
	p_cmd.apns_production  = p_file.apns_production;
	p_cmd.override_msg_id  = p_file.override_msg_id;
	p_cmd.send_description = p_file.send_description;  
	p_cmd.msg_type		   = p_file.msg_type;
}

void jpush_arg::store_cmd_data(fifo_jpush_data& p_cmd)
{
	 sem_wait(&g_sem_empty);
	 pthread_mutex_lock(&g_mutex);
		cmd_head.push_back(p_cmd);
	 pthread_mutex_unlock(&g_mutex);
	 sem_post(&g_sem_full);
}

void jpush_arg::config_value(const char *value_src,const char *keyword,fifo_jpush_data& p_cmd)
{
	if (!strcasecmp(keyword,"sendno"))
		p_cmd.sendno = strtol(value_src,NULL,10);
	else if (!strcasecmp(keyword,"app_key")) 
		p_cmd.app_key = value_src;
	else if(!strcasecmp(keyword,"receiver_type"))
		p_cmd.receiver_type = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"receiver_value"))
		p_cmd.receiver_value = value_src;
	else if(!strcasecmp(keyword,"verification_code"))
		p_cmd.verification_code = value_src;
	else if(!strcasecmp(keyword,"msg_type"))
		p_cmd.msg_type = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"RegistrationID"))
		p_cmd.RegistrationID = value_src;
	else if(!strcasecmp(keyword,"n_builder_id"))
		p_cmd.msg_content.n_builder_id = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"n_title"))
		p_cmd.msg_content.n_title = value_src;
	else if(strstr(keyword,"n_extras"))
	{
		static fifo_jpush_data::extras_data dataTemp;
		if(strstr(keyword,"key"))
		{
			dataTemp.key = value_src;
		}
		else if(strstr(keyword,"value"))
		{
			dataTemp.value = value_src;
			p_cmd.msg_content.n_extras.push_back(dataTemp);
			dataTemp.key = "";
			dataTemp.value = "";
		}
		
			return ;
	}
	else if(!strcasecmp(keyword,"n_content"))
		p_cmd.msg_content.n_content = value_src;
	else if(!strcasecmp(keyword,"send_description"))  
		p_cmd.send_description = value_src;
	else if(!strcasecmp(keyword,"platform"))
		p_cmd.platform = value_src;
	else if(!strcasecmp(keyword,"apns_production"))
		p_cmd.apns_production = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"time_to_live"))
		p_cmd.time_to_live = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"override_msg_id"))
		p_cmd.override_msg_id = value_src;
	else if(!strcasecmp(keyword,"record_name"))
		p_cmd.record_name = value_src;  
	else if(!strcasecmp(keyword,"master_secret"))
		p_cmd.master_secret = value_src;
		else if(!strcasecmp(keyword,"customizedID"))  
		p_cmd.customizedID = value_src;
	else if(!strcasecmp(keyword,"db_ip"))
		run_arg.db_par_.ip = value_src;
	else if(!strcasecmp(keyword,"db_port"))
		run_arg.db_par_.port = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"db_dbname"))
		run_arg.db_par_.dbname = value_src;
	else if(!strcasecmp(keyword,"db_loginname"))
		run_arg.db_par_.loginname = value_src;
	else if(!strcasecmp(keyword,"db_password"))
		run_arg.db_par_.password = value_src;
	else if(!strcasecmp(keyword,"db_connect_timeout"))
		run_arg.db_par_.time_out = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"db_table_name"))
		run_arg.db_par_.db_table_name = value_src;
	else if(!strcasecmp(keyword,"redis_host"))
		run_arg.redis_par_.redis_host = value_src;
	else if(!strcasecmp(keyword,"redis_port"))
		run_arg.redis_par_.redis_port = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"redis_dbindex"))
		run_arg.redis_par_.dbindex = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"set_log_level"))
		run_arg.log_par_.log_level = value_src;
	else if(!strcasecmp(keyword,"set_log_name"))
		run_arg.log_par_.log_name = value_src;  
	else if(!strcasecmp(keyword,"enable_syslog"))
		run_arg.log_par_.enable_syslog = value_src;  
	else if(!strcasecmp(keyword,"set_log_path"))
		run_arg.log_par_.file_log_path = value_src;  
	else if(!strcasecmp(keyword,"enable_console"))
		run_arg.log_par_.enable_console = value_src;  
	else if(!strcasecmp(keyword,"thread_num"))
		run_arg.thread_num = strtol(value_src,NULL,10);
	else if(!strcasecmp(keyword,"redis_primary_key"))
		run_arg.redis_primary_key = value_src; 
	else if(!strcasecmp(keyword,"enable_daemon"))
		run_arg.enable_daemon = value_src; 
	return ;
}


void jpush_arg::get_value(jpush_arg::fifo_jpush_data& temp)
{
	/*
	if(check_cmd_empty() == true)
	{
			event_logger(LOG_DEBUG,"cmd list is empty,....");
					return false;
	}
	*/
	sem_wait(&g_sem_full);
	pthread_mutex_lock(&g_mutex);
		temp = cmd_head.front();
		del_cmd_head();
	pthread_mutex_unlock(&g_mutex);
	sem_post(&g_sem_empty);
}
void jpush_arg::del_cmd_head()
{
	cmd_head.pop_front();
}

bool jpush_arg::check_cmd_empty()
{
	return cmd_head.empty();
}
jpush_arg::runtime_parameter& jpush_arg::get_runtime_value()
{
	return run_arg;
}

bool jpush_arg::reset_value()
{
	cmd_head.clear();
}
int jpush_arg::is_data_ok(fifo_jpush_data& p_cmd)
{

	//		  (p_cmd.msg_content.n_content !="") &&  
	   if(p_cmd.sendno >0 && \
	      (p_cmd.app_key != "") && \
		  (p_cmd.receiver_type ==2 || p_cmd.receiver_type ==3 || p_cmd.receiver_type ==4 || p_cmd.receiver_type ==5) && \
		  (p_cmd.msg_type == 1 || p_cmd.msg_type == 2) && \
		  (strstr(p_cmd.platform.c_str(),"android") || strstr(p_cmd.platform.c_str(),"ios") || strstr(p_cmd.platform.c_str(),"winphone"))
		  )
		{
		  
			return 1;
		}
	
	else 
		return 0;
}
void jpush_arg::print_value(fifo_jpush_data& p_cmd)
{
	cout<<"sendno -- "<<p_cmd.sendno<<endl;
	cout<<"app_key --"<<p_cmd.app_key<<endl;
	cout<<"receiver_type -- "<<p_cmd.receiver_type<<endl;
	cout<<"receiver_value -- "<<p_cmd.receiver_value<<endl;
	cout<<"verification_code -- "<<p_cmd.verification_code<<endl;
	cout<<"msg_type -- "<<p_cmd.msg_type<<endl;
	
	
	cout<<"msg_content:"<<endl;
	cout<<"n_builder_id -- "<<p_cmd.msg_content.n_builder_id<<endl;
	cout<<"n_title -- "<<p_cmd.msg_content.n_title<<endl;
	cout<<"n_content -- "<<p_cmd.msg_content.n_content<<endl;
	
	vector<fifo_jpush_data::extras_data>::iterator startIterator = p_cmd.msg_content.n_extras.begin();
	vector<fifo_jpush_data::extras_data>::iterator endIterator = p_cmd.msg_content.n_extras.end();
	
	cout<<"n_extras:"<<endl;
	int count = 0;
	while(startIterator != endIterator)
	{
		count++;
		cout<<"  "<<"n_extras["<<count<<"]_key: "<<startIterator->key<<"   "<<"n_extras["<<count<<"]_value: "<<startIterator->value<<endl;
		startIterator ++;
	}
	cout<<"send_description -- "<<p_cmd.send_description<<endl;
	cout<<"platform -- "<<p_cmd.platform<<endl;
	cout<<"apns_production -- "<<p_cmd.apns_production<<endl;
	
	cout<<"time_to_live -- "<<p_cmd.time_to_live<<endl;
	cout<<"override_msg_id -- "<<p_cmd.override_msg_id<<endl;
	cout<<"record_name -- "<<p_cmd.record_name<<endl;



	cout<<"=========redis parameter=============="<<endl;
	cout<<"redis_primary_key -- "<<run_arg.redis_primary_key<<endl;
	cout<<"record_ip -- "<<run_arg.redis_par_.redis_host<<endl;
	cout<<"record_port -- "<<run_arg.redis_par_.redis_port<<endl;
	cout<<"redis_dbindex -- "<<run_arg.redis_par_.dbindex<<endl;
	cout<<"redis_primary_key -- "<<run_arg.redis_primary_key<<endl;
	cout<<endl;
	cout<<"=========mysql connect parameter============"<<endl;
	cout<<"mysql connect port -- "<<run_arg.db_par_.port<<endl;
	cout<<"mysql connect dbname -- "<<run_arg.db_par_.dbname<<endl;
	cout<<"mysql connect loginname -- "<<run_arg.db_par_.loginname<<endl;
	cout<<"mysql connect password -- "<<run_arg.db_par_.password<<endl;
	cout<<"mysql connect timeout -- "<<run_arg.db_par_.time_out<<endl;
}













