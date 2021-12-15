#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/msg.h> // for message queue
#include <sys/ipc.h> // for message queue
#include <sys/types.h> // for message queue
#include "msg_data.h" // define struct message & real_data
#include <time.h>

#define NUMTHRDS 6 // thread Num
#define BILLION 1000000000L;

pthread_t callThd[NUMTHRDS]; // thread 3
pthread_mutex_t dbmutex; // DB MUTEX


void* t_function();
void* query_execute();
void* query_insert(void *msg); // insert query
MYSQL conn_ptr;        //connect DB
MYSQL_RES* res;        //result
MYSQL_ROW row;
    
    
int main(int argc, char* agrv[])
{
    pthread_attr_t attr;
    int i;
    void* status;
    int  tid;
    key_t key1 = 60090; // for message queue's key value
    key_t key2 = 60091; // for message queue's key value
    int msqid1; // message queue id
    int msqid2; // message queue id
    struct message msg[6]; // message
    struct timespec start,stop; // 사용한 IPC 기법에 따른 성능 차이를 측정하기 위해서
    double accum; // 시간 측정값 받기 위한 변수

    if((msqid1=msgget(key1,IPC_CREAT|0666))== -1){ // msqid get message queue id
	    printf("message receive failed\n");
    }
    
    if((msqid2=msgget(key2,IPC_CREAT|0666))== -1){ // msqid get message queue id
            printf("message receive failed\n");
    }


    if(clock_gettime(CLOCK_MONOTONIC,&start) == -1){ // Message IPC 기법을 이용해서 Message 를 Receive 할 때, 걸리는 시간 측정 위해서 start
		perror("clock gettime");
		pthread_exit(NULL);
	}
    for(i=0;i<3;i++){
    	if(msgrcv(msqid1, &msg[i], sizeof(struct real_data),1,0) == -1){ // if msgrcv failed, then exit
        	printf("msgrcv failed\n");
        	exit(0);
    	}else{
    		printf("msgrcv is successed\n");
        	printf("sequence: %d, user ID: %s, password : %s, sex: %s, mobile: %s, email: %s\n",msg[i].data.sequence, msg[i].data.userID, msg[i].data.password, msg[i].data.sex, msg[i].data.mobile, msg[i].data.email);
		
    	}
    	
    }

    for(i=3;i<6;i++){
        if(msgrcv(msqid2, &msg[i], sizeof(struct real_data),1,0) == -1){ // if msgrcv failed, then exit
                printf("msgrcv failed\n");
                exit(0);
        }else{
                printf("msgrcv is successed\n");
                printf("sequence: %d, user ID: %s, password : %s, sex: %s, mobile: %s, email: %s\n",msg[i].data.sequence, msg[i].data.userID, msg[i].data.password, msg[i].data.sex, msg[i].data.mobile, msg[i].data.email);

        }

    }
    if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1){ // Message IPC 기법을 이용해서 Message 를 Receive 할 때, 걸리는 시간 측정 위해서 stop
		perror("clock gettime");
		pthread_exit(NULL);
	}



    accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
    printf("All message received\n");
    printf("All 메시지 receive하는데 걸리는 시간 : %.9f\n",accum);

    if(msgctl(msqid1,IPC_RMID,NULL)==-1){ // if message queue delete's failed, then exit
    	printf("msgctl falied\n");
	exit(0);
    }

    if(msgctl(msqid2,IPC_RMID,NULL)==-1){ // if message queue delete's failed, then exit
        printf("msgctl falied\n");
        exit(0);
    }


    
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
		pthread_join(callThd[i],&status);
	} 
	pthread_mutex_destroy(&dbmutex); // destroy
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

void* query_execute(){// we will get argument about 1: insert 2: delete 3: update 4: read
	sleep(1);
	int fields;
	int cnt;
	pid_t pid;            // process id
    	pthread_t tid;        // thread id
	
	pid = getpid();
    	tid = pthread_self();
 
    	int i = 0;
    
    	while (i<3)   // 0,1,2 까지만 loop 돌립니다.
    	{
        	// 넘겨받은 쓰레드 이름과 
        	// 현재 process id 와 thread id 를 함께 출력
        	printf("pid:%u, tid:%x --- %d\n", 
            	(unsigned int)pid, (unsigned int)tid, i);
        	i++;
        	sleep(1);  // 1초간 대기
    	}
	
	pthread_mutex_lock(&dbmutex);
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
        pthread_mutex_unlock(&dbmutex);
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
