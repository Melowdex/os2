TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

#-lpthread toevoegen om te compilen
# when executing make, compile all executables
all: main 

main : main.c 
	@mkdir -p build
	@echo "$(TITLE_COLOR)\n***** COMPILING main.c *****$(NO_COLOR)"
	gcc -c main.c -lpthread -Wall -Werror -o build/main.o -g
	@echo "$(TITLE_COLOR)\n***** LINKING main *****$(NO_COLOR)"
	gcc build/main.o splist.o -ltcpsock -o server -Wall -L./lib -Wl,-rpath=./lib

clean:
	rm -rf build main *~

run : server client_node
	@echo running the server
	./server

debug-server: server
	@echo "$(TITLE_COLOR)\n***** Running GDB on server *****$(NO_COLOR)"
	gdb -q -ex run --args ./server

debug-client: client_node
	@echo "$(TITLE_COLOR)\n***** Running GDB on client_node *****$(NO_COLOR)"
	gdb -q -ex run --args ./client_node 2 127.0.0.1 5678

hfd: hfd.c
	gcc -c hfd.c -Wall -Werror -o build/hfd.o -g
	gcc build/hfd.o -ltcpsock -o hfd -Wall -L./lib -Wl,-rpath=./lib

run-hfd:
	./hfd