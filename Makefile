all:
	gcc -Wall -c common.c
	gcc -Wall -lpthread equipment.c common.o -lpthread -o equipment
	gcc -Wall -lpthread server.c common.o -lpthread -o server

clean:
	rm common.o equipment server