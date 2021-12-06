#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "msg_data.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#define NUMTHRDS 3 // 쓰레드 개수
#define BILLION 1000000000L;

pthread_t callThd[NUMTHRDS]; // 쓰레드 3개 생성 할 것임
pthread_mutex_t dbmutex; // DB MUTEX


int sequence = 3; // sequence number
struct message join();
void menu(); // menu 보여주는 함수
void* mysql_query(void *msg);


int main(){
    pthread_attr_t attr;
    void* status;
    int tid;
    int i;
    int num;
    struct message msg[3]; // message 구조체 배열
    
    if(mkfifo("./FIFO2",0666) == -1){
            printf("fail to call fifo()\n");
    }


    for(i = 0; i<3; i++){ //삽입할  회원정보 3번 기입
        menu();
	scanf("%d",&num);
	if(num == 1){
             msg[i] = join(); // 세번 가입하여 msg[i] 에 3개 저장
	}else if(num ==2){
		return 0;
	}else{
		printf("try again");
		continue;
	}

    }
    pthread_mutex_init(&dbmutex, NULL); // Initialize mutex    
    pthread_attr_init(&attr); // pthread attribute's initialize
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // pthread attribute SET JOINABLE

    for(i=0; i<NUMTHRDS; i++){ // thread 3 created
        tid = pthread_create(&callThd[i], &attr,mysql_query, (void*)&msg[i]);
        if(tid < 0){
            perror("thread create error : ");
            exit(0);
        }
    }

    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&dbmutex); // destroy
    for(i=0; i<NUMTHRDS; i++){
        pthread_join(callThd[i], (void**)status);
    }
    pthread_exit(NULL);

    return 0;
}

void menu(){
    printf("==================\n");
        printf("1.회원가입\n");
        printf("2.종료\n");
        printf("==================\n");          
        printf("\n");
}

void* mysql_query(void *arg){
    int fp; //file pointer
    struct message *msg = (struct message*)arg;
    char buff[sizeof(*msg)];
    struct timespec start,stop; // 사용한 IPC 기법에 따른 성능 차이를 측정하기 위해서
    double accum; // 시간 측정값 받기 위한 변수
    
    pthread_mutex_lock(&dbmutex);
    msg->data.sequence = ++sequence;
    printf("sequence: %d\n",sequence); 
    pthread_mutex_unlock(&dbmutex);
    if((fp = open("./FIFO2", O_WRONLY)) < 0){
	    printf("file open error\n");
	    exit(1);
    }else{
	    sprintf(buff,"%d,%s,%s,%s,%s,%s",msg->data.sequence,msg->data.userID,msg->data.password,msg->data.sex,msg->data.mobile,msg->data.email);
	    if(clock_gettime(CLOCK_MONOTONIC,&start) == -1){ // Named PIPE 기법을 이용해서 Message 를 Send 할 때, 걸리는 시간 측정 위해서 start
		perror("clock gettime");
		pthread_exit(NULL);
	    }
	    write(fp, buff, sizeof(buff));
	    if(clock_gettime(CLOCK_MONOTONIC, &stop) == -1){ // Named PIPE 기법을 이용해서 Message 를 Send 할 때, 걸리는 시간 측정 위해서 stop
		perror("clock gettime");
		pthread_exit(NULL);
	    }
	    printf("%s\n",buff);
	    accum = (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / (double)BILLION;
	    printf("write message in PIPE gets time : %.9f\n",accum);
	    printf("%s\n",buff);
    }
    close(fp);
    printf("message sent\n");
}

struct message join(){
    struct message msg; // message qeue 키 구조체
    struct real_data data; // 실제 데이터의 구조체
    while(1)
    {
        printf("1) 아이디를 입력하시오 :");
        scanf("%s",data.userID);
        if(data.userID == NULL){ // userID NOT NULL 이기 때문에
            printf("ID는 꼭 입력해야 합니다.\n");
            continue;
        }
        printf("\n");
        printf("2) 비밀번호를 입력하시오 :");
        scanf("%s",data.password);
        if(data.password == NULL){ // password NOT NULL 이기 때문에
            printf("비밀번호는 꼭 입력해야 합니다.\n");
            continue;
        }
        printf("\n");
        printf("3) 성별을 입력하시오 : (MEN or WOMEN)");
        while(1){
            scanf("%s",data.sex);
            if(!strcmp(data.sex,"MEN")){
            	break;
            }else if(!strcmp(data.sex,"WOMEN")){
            	break;
            }else printf("try again\n");
        }
        printf("\n");
        printf("4) 전화번호를 입력하시오 :");
        scanf("%s",data.mobile);
        printf("\n");
        printf("5) 이메일을 입력하시오: ");
        scanf("%s",data.email);
        printf("\n");
        break;
    }

    strcpy(msg.data.userID, data.userID);
    strcpy(msg.data.password, data.password);
    strcpy(msg.data.sex,data.sex);
    strcpy(msg.data.mobile,data.mobile);
    strcpy(msg.data.email,data.email);
    

    return msg;
}
