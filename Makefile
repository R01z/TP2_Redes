all:
	gcc -Wall -c racks.c
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o client
	gcc -Wall server.c common.o racks.o -o server