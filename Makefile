all:	client  server

clean:
	rm -f server client thread_file

client: client2.c
	gcc -std=c99 -pthread client2.c -o client -lncurses

server: server.c
	gcc -std=c99 -pthread server.c -o server


