.PHONY:clean all

all: server client
	@echo "Both server and client have been built."

client: ./client/main.o
	gcc ./client/main.o -lpthread -o client.elf
	@echo "Client has been built."

./client/main.o: ./client/main.c
	gcc -c ./client/main.c -o ./client/main.o

server: ./server/main.o
	gcc ./server/main.o -lpthread -o  server.elf
	@echo "Server has been built."

./server/main.o: ./server/main.c
	gcc -c ./server/main.c -o ./server/main.o

clean:
	-rm -rf ./client/*.o
	-rm -rf ./client/client
	-rm -rf ./server/*.o
	-rm -rf ./server/server
	-rm -rf server.elf  client.elf