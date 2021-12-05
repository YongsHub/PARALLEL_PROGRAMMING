#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include "msg_data.h"
#include <pthread.h>

#define NUMTHRDS 3 // 쓰레드 개수
pthread_t callThd[NUMTHRDS]; // 쓰레드 3개 생성 할 것임
pthread_mutex_t dbmutex; // DB MUTEX


int sequence = 0; // sequence number
struct message join();
void printMsgInfo(int msqid); // 메시지 큐 이용하여 메시지 큐 정보
void menu(); // menu 보여주는 함수
void* mysql_query(void *msg);


int main(){
    pthread_attr_t attr;
    void* status;
    int tid;
    int i;
    int num;
    struct message msg[3]; // message 구조체 배열


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


void printMsgInfo(int msqid){
        struct msqid_ds m_stat;
        printf("========== messege queue info =============\n");
        if(msgctl(msqid,IPC_STAT,&m_stat)==-1){
                printf("msgctl failed");
                exit(0);
        }
        printf(" message queue info \n");
        printf(" msg_lspid : %d\n",m_stat.msg_lspid);
        printf(" msg_qnum : %ld\n",m_stat.msg_qnum);
        printf(" msg_stime : %ld\n",m_stat.msg_stime);

        printf("========== messege queue info end =============\n");
}


void menu(){
    printf("==================\n");
        printf("1.회원가입\n");
        printf("2.종료\n");
        printf("==================\n");          
        printf("\n");
}

void* mysql_query(void *arg){
    key_t key = 60090; // IPC KEY 값
    struct message *msg = (struct message*)arg;
    int msqid; // message queue id

    if((msqid = msgget(key, IPC_CREAT|0666)) == -1){
        printf("msgget failed\n");
        exit(0);
    }
    msg->msg_type = 1; // insert
    pthread_mutex_lock(&dbmutex);
    msg->data.sequence = ++sequence;
    printf("sequence: %d\n",sequence);
    pthread_mutex_unlock(&dbmutex); 
    printMsgInfo(msqid);
    if(msgsnd(msqid, msg, sizeof(struct real_data),0) == -1){
        printf("msgnd failed\n");
        exit(0);
    }

    printf("message sent\n");
    printMsgInfo(msqid);
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
        printf("3) 성별을 입력하시오 : ");
        scanf("%s",data.sex);
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
