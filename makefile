Server-Client: server client

server: server.c 
	gcc server.c -o server -lpthread -g
client: client.c
	gcc client.c -o client -lpthread -g
