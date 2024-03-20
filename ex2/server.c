/**
 * @author Xander Verberckt
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "config.h"
#include "lib/tcpsock.h"

#define PORT 3000
#define MAXBUFFER 100

void loggerCode();
void serverCode();

static void SIGINT_handler(int sig);

static volatile int running = 1;
int pipefd[2];
char buf[MAXBUFFER];

int main(){

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int processLogger = fork();

    if (processLogger == 0){
        loggerCode();
    } else{
        serverCode();
    }
    return 0;
}

void serverCode(){

    struct sigaction sa;
    sa.sa_handler = SIGINT_handler;
    sigaction(SIGINT, &sa, NULL);

    close(pipefd[0]);

    write(pipefd[1], "Logger started!\n", strlen("Logger started!\n"));

    tcpsock_t *server, *client;

    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);

    printf("Server is started\n");
    write(pipefd[1], "Server started!\n", strlen("Server started!\n"));

    while (running == 1){

    }

    close(pipefd[1]);
    wait(NULL);
    exit(EXIT_SUCCESS);
}

void loggerCode(){
    int i = 0;
    int rres;
    close(pipefd[1]);
    FILE* f = fopen("log.txt", "w");
    
    do {
        rres = read(pipefd[0], buf, MAXBUFFER);
        if (rres > 0){
            fprintf(f, "%d) %s", i, buf);
            ++i;
        }
    } while (rres > 0);
    
    fclose(f);
    close(pipefd[0]);
    _exit(EXIT_SUCCESS);
}

static void SIGINT_handler(int sig) {
    printf("signal received by application\n");
    running = 0;
}