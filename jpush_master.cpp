#include "redisclient.h"
#include "event_drive.h"

int main(int argc,char **argv)
{
  if(argc < 2)
  {
	  cout<<"  [ usag: ] you must assign config file path"<<endl;
	  return -1;
  }
  else
  {
	event_handle ev(argv[1]);  
	//ev.store_data_redis();
	ev.event_run();  
  }	  
  return 0;
}



  