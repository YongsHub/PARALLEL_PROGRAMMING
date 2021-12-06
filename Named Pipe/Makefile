all: client1 client2 server

CC = gcc

client1: client1.c
	$(CC) -o $@ $< -lpthread
client2: client2.c
	$(CC) -o $@ $< -lpthread

server: server.c
	$(CC) -o $@ $< -lpthread -lmysqlclient
