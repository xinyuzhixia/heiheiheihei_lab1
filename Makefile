all: server/server.c deliver/deliver.c
	gcc -g -o server/server server/server.c -I.
	gcc -g -o deliver/deliver deliver/deliver.c -I.
