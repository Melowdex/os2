TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

# when executing make, compile all executables
all: server client_node 

server : server.c dataworker.c lib/libtcpsock.so
	@mkdir -p build
	@echo "$(TITLE_COLOR)\n***** COMPILING server *****$(NO_COLOR)"
	gcc -c server.c -Wall -Werror -o build/server.o -g
	gcc -c dataworker.c -Wall -Werror -o build/dataworker.o -g
	gcc -c sbuffer.c -lpthread -Wall -Werror -o build/sbuffer.o -g
	@echo "$(TITLE_COLOR)\n***** LINKING server *****$(NO_COLOR)"
	gcc build/server.o build/dataworker.o build/sbuffer.o splist.o -ltcpsock -lpthread -lssl -lcrypto -o server -Wall -L./lib -Wl,-rpath=./lib

client_node : client_node.c lib/libtcpsock.so
	@mkdir -p build
	@echo "$(TITLE_COLOR)\n***** COMPILING client_node *****$(NO_COLOR)"
	gcc -c client_node.c -Wall -Werror -o build/client_node.o -D LOOPS=3 -g
	@echo "$(TITLE_COLOR)\n***** LINKING client_node *****$(NO_COLOR)"
	gcc build/client_node.o -ltcpsock -o client_node -Wall -L./lib -Wl,-rpath=./lib

# If you only want to compile one of the libs, this target will match (e.g. make libtcpsoc)
libtcpsock : lib/libtcpsock.so

lib/libtcpsock.so : lib/tcpsock.c
	@mkdir -p build
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB tcpsock *****$(NO_COLOR)"
	gcc -c lib/tcpsock.c -Wall -Werror -fPIC -o build/tcpsock.o -g
	@echo "$(TITLE_COLOR)\n***** LINKING LIB tcpsock *****$(NO_COLOR)"
	gcc build/tcpsock.o -o lib/libtcpsock.so -Wall -shared -lm

# do not look for files called clean, clean-all or this will be always a target
.PHONY : clean clean-all run

clean:
	rm -rf build server client_node *~

clean-all: clean
	rm -rf lib/*.so

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