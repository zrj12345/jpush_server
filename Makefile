#CFLAGS?= -pedantic -O2 -Wall -DNEBUG -W
CFLAGS?= -pedantic -O0 -W -DDEBUG -g 
GPLUSPLUS = g++

CLIBSQL = -I/usr/include/ -L/usr/lib64/mysql -lmysqlclient

BIN = JpushMaster 
CLIBS = -I/usr/include -L/usr/lib64/ 

LINKS = -lboost_thread -lpthread -lcurl -lcrypto  
TESTAPPOBJS = anet.o DbPool.o DbRecordset.o DbConnect.o files.o parson.o pthread_handle.o event_drive.o



$(BIN):$(TESTAPPOBJS) 
	$(GPLUSPLUS) jpush_master.cpp -o $@   $(CLIBSQL) $(LINKS) $(CFLAGS) $^
%.o:%.c
	$(CC) -c $< -o $@

%.o:%.cpp
	$(GPLUSPLUS) $(CLIBSQL) -c $< -o $@
	



clean:
	rm -rf *.o  *.bak
	#rm test_client.o 

dep:
	$(CC) -MM *.c *.cpp

log:
	git log '--pretty=format:%ad %s' --date=short > Changelog



#g++ test_client.cpp -o main -L /usr/lib64 -lboost_thread -lpthread -lcurl -lcrypto

app:
	g++ -c anet.c 
	g++ -c DbRecordset.cpp -I/usr/include/ -L/usr/lib64/mysql -lmysqlclient
	g++ -c DbPool.cpp -I/usr/include/ -L/usr/lib64/mysql -lmysqlclient
	g++ -c DbConnect.cpp -I/usr/include/ -L/usr/lib64/mysql -lmysqlclient
	g++ test_client.cpp -o main -I/usr/include/ -L/usr/lib64/mysql -lmysqlclient -lboost_thread -lpthread -lcurl -lcrypto  anet.o DbPool.o DbRecordset.o DbConnect.o
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
