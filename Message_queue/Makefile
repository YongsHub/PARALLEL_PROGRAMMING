all: client1 client2 server

CC = gcc

client1: message_queue_client1.c
	$(CC) -o $@ $< -lpthread
client2: message_queue_client2.c
	$(CC) -o $@ $< -lpthread

server: message_queue_server.c
	$(CC) -o $@ $< -lpthread -lmysqlclient
