Server-Client: server client

server: server.c bitrate.h
	gcc server.c -o server -lpthread -g
client: client.c
	gcc client.c -o client -lpthread -g
