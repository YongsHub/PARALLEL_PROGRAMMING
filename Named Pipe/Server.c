#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "msg_data.h" // define struct message & real_data


#define NUMTHRDS 6 // thread Num
pthread_t callThd[NUMTHRDS]; // thread 3
pthread_mutex_t dbmutex; // DB MUTEX


void* t_function();
void query_execute();
void* query_insert(void *msg); // insert query
void message_insert(int loop, char *fileName);
MYSQL conn_ptr;        //connect DB
MYSQL_RES* res;        //result
MYSQL_ROW row;
    
struct message msg[6];
int main(int argc, char* agrv[])
{
    pthread_attr_t attr;
    void* status;
    int  tid;
    int i;
    
    
    message_insert(3,"./FIFO1"); // Named Pipe: FIFO1 Routine 
    message_insert(6,"./FIFO2"); // NAMED pIPE: FIFO2 Routine
    pthread_mutex_init(&dbmutex, NULL); // Initialize mutex
    pthread_attr_init(&attr); // pthread attribute's initialize to attr object
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    


    mysql_init(&conn_ptr);    //mssql initialize   

    
    if(!mysql_real_connect(&conn_ptr, "127.0.0.1", "root", "5201", "unix_user", 0, NULL, 0))// if user disconnect to DATABASE, exit(1)
    {
        printf("%s\n", mysql_error(&conn_ptr));
        exit(1);
    }


        /*IN here, we need to make Multi-Thread like 0: Insert, 1: Delete 2: Update 3: Read */
        for(i=0; i<NUMTHRDS; i++){//Thread NUM MAKES LOOP
       		 tid = pthread_create(&callThd[i], &attr,query_insert,(void*)&msg[i]);
       		 if(tid < 0){
               		 perror("thread create error : ");
               		 exit(0);
         	 }
       }

        
        pthread_attr_destroy(&attr);

	for(i=0; i<NUMTHRDS; i++){ // it makes all threads wait for execute's all
		pthread_join(callThd[i], (void**)status);
	} 
	pthread_mutex_destroy(&dbmutex); // destroy
	printf("PRINT DATABASE\n");
	query_execute();
	unlink("./FIFO1"); // rm FIFO1
	unlink("./FIFO2"); // rm FIFO2
	pthread_exit(NULL);
	mysql_free_result(res);
	mysql_close(&conn_ptr);
        return 0;
}

void *t_function()
{
    pid_t pid;            // process id
    pthread_t tid;        // thread id
 
    pid = getpid();
    tid = pthread_self();
 
    int i = 0;
    
    pthread_mutex_lock(&dbmutex);
    while (i<3)   // 0,1,2 까지만 loop 돌립니다.
    {
        // 넘겨받은 쓰레드 이름과 
        // 현재 process id 와 thread id 를 함께 출력
        printf("pid:%u, tid:%x --- %d\n", 
            (unsigned int)pid, (unsigned int)tid, i);
        i++;
        sleep(1);  // 1초간 대기
    }
    pthread_mutex_unlock(&dbmutex);
}

void query_execute(){// READ DATABASE
	sleep(1);
	int fields;
	int cnt;
	
	if(mysql_query(&conn_ptr, "select * from users")) // query start
    	{
        	printf("%s\n", mysql_error(&conn_ptr)); // if it makes error, then exit
        	exit(1);
    	}

        //receive result value at once
        res = mysql_store_result(&conn_ptr);

        //get fields numbers
        fields = mysql_num_fields(res);

        //get 1 ROW in result, field is array type
        while((row = mysql_fetch_row(res))) 
        {
            for(cnt = 0; cnt<fields; ++cnt)
                printf("%s  ", row[cnt]); // userID, password, sex, mobile, email printing

            printf("\n");
        }

        //mysql_free_result(res);
        //mysql_close(&conn_ptr);    //close server, memory free
}

void * query_insert(void *arg){
	struct message *msg = (struct message*)arg;
        char buff[500];
	
	pthread_mutex_lock(&dbmutex);
   	sprintf(buff,"insert into users values""('%d','%s','%s','%s','%s','%s')",msg->data.sequence,msg->data.userID,msg->data.password,msg->data.sex,msg->data.mobile,msg->data.email);
	if(mysql_query(&conn_ptr,buff)) // query start insert into users
  	{
        	 printf("%s\n", mysql_error(&conn_ptr)); // if it makes error, then exit
   		 exit(1);
        }
	else
	{
  		 printf("DATA INSERT SUCCESS\n");
	 	 sleep(1);
   	}
	 pthread_mutex_unlock(&dbmutex);
}

void message_insert(int loop, char *fileName){
    int fp;
    int i;
    int j=0;
    char buff[sizeof(msg[0])];
    
    
    if((fp=open(fileName,O_RDONLY)) < 0){
    	printf("open fifo failed\n");
    	exit(1);
    }
    if(loop == 3){ // seq 0,1,2 .... so loop = 3
    	i = 0;
    }else if(loop == 6){ // seq 3,4,5 so loop = 6
    	i = 3;
    }
    	
    for(;i<loop;i++){
    	if(read(fp,buff,sizeof(buff))> 0){
    		printf("receive is success\n");
		char *ptr = strtok(buff, ",");
		while(ptr != NULL){
			printf("%s\n",ptr);
			if(j==0){
				msg[i].data.sequence =atoi(ptr);
			}else if(j==1){
				strcpy(msg[i].data.userID,ptr);
				//msg[i].data.userID = ptr;
			}else if(j==2){
				strcpy(msg[i].data.password,ptr);
				//msg[i].data.password = ptr;
			}else if(j==3){
				strcpy(msg[i].data.sex,ptr);
				//msg[i].data.sex = ptr;
			}else if(j==4){
				strcpy(msg[i].data.mobile,ptr);
				//msg[i].data.mobile = ptr;
			}else if(j==5){
				strcpy(msg[i].data.email,ptr);
				//msg[i].data.email = ptr;
			}
			j++;
			ptr = strtok(NULL,",");
		}
		j=0;
	}
    }
}
