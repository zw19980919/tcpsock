.PHONY: clean all

client: main.o  
	gcc  main.o  -lpthread -o  client  
	@echo "Client has been built."  
  
main.o: main.c  
	gcc -c main.c -o main.o  
  
all: client  
	@echo "All done."  
  
clean:  
	-rm -f main.o client