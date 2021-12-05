struct real_data{
	char userID[50];
	char password[50];
	char sex[10];
	char mobile[50];
	char email[50];
	int sequence;
};

struct message{
	long msg_type;// 0 is insert into MYSQL DATABASE;
	struct real_data data;
};

