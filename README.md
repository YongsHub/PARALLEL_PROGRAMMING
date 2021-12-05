# PARALLEL_PROGRAMMING
SERVER-CLIENT모델간 IPC기법을 활용하여 병렬적 응용프로그램 만들기

응용 프로그램1
서버 프로세스 1개 클라이언트 프로세스 2개
각 프로세스 당 쓰레드는 3개씩 생성되며, MYSQL DATABASE를 활용하여 회원가입 프로그램 만들기
NAMED PIPE를 활용


응용 프로그램2
위와 같지만 프로그램2는 고급IPC 기법중 Message Queue를 활용하여 프로세스간 통신함.


