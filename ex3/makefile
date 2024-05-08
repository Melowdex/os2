TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

#-lpthread toevoegen om te compilen
# when executing make, compile all executables
all: main 

main : main.c sbuffer.c
	@mkdir -p build
	@echo "$(TITLE_COLOR)\n***** COMPILING main.c *****$(NO_COLOR)"
	gcc -c main.c -lpthread -lssl -lcrypto -Wall -Werror -o build/main.o -g
	gcc -c sbuffer.c -lpthread -Wall -Werror -o build/sbuffer.o -g
	@echo "$(TITLE_COLOR)\n***** LINKING main *****$(NO_COLOR)"
	gcc build/main.o build/sbuffer.o -lpthread -lssl -lcrypto -o main -Wall -L./lib -Wl,-rpath=./lib

clean:
	rm -rf build main *~

run : main
	@echo running the server
	./main

debug: main
	@echo "$(TITLE_COLOR)\n***** Running GDB on main *****$(NO_COLOR)"
	gdb -q -ex run --args ./main 

valgrind : main
	@echo "$(TITLE_COLOR)Running Valgrind :$(NO_COLOR)"
	CK_FORK=no valgrind --leak-check=full --track-origins=yes ./main