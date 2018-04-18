default: server

server: server.c
	gcc server.c -o server

clean:
	-rm -f server.o server
