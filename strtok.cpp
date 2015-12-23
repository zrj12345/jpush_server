#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
using namespace std;
/*
bool string_filter(char *src,const char* delim,vector<string>& res)
{
	if(src == NULL || delim == NULL)  return false;
	
	while(strstr(src,",,"))
	{
		
	}
	string first;
	string temp;
	char *temp1 = (char *)malloc(strlen(src));
	memcpy(temp1,src,strlen(src));
	char *p = NULL;
	first = strtok(src,delim);
	cout<<"first:"<<first<<endl;
	cout<<"src:"<<src<<endl;
	if(strstr(src,delim) == src)		
		res.push_back("");
	res.push_back(first);

		
	while(p = strtok(NULL,delim))
	{
		printf("p:%s\n",p);
		cout<<"src:"<<src<<endl;
		temp = p;
		res.push_back(temp);
	}
		
	char *ptr = temp1 + strlen(temp1) - strlen(delim);
	printf("*temp1:%c\n",*temp1);
	if(strstr(ptr,delim) == ptr)
		res.push_back("");
	free(temp1);
	return true;
}
*/


//"123,world,,yes,123456,";
void string_filter(char *src,const char* delim,vector<string>& res)
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

int main()
{
	
	vector<string> res;	
	char src[128] = ",world,,yes,123456,qqqqqa";
	
	string_filter(src,",",res);	

	for(int i = 0;i<res.size();i++)	
	{
		cout<<i<<":"<<res.at(i)<<"  ";
	}
		cout<<endl;	
	return 0;
}

