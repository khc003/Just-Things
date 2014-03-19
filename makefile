all: first second

first: server
	./server MessageBot 3490
second: server.c
	gcc -o server server.c -lpthread

