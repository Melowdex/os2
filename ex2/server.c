/**
 * @author Xander Verberckt
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "splist.h"
#include "config.h"
#include "lib/tcpsock.h"

#define PORT 3000
#define MAXBUFFER 100
#define MAXTEXT 60

void loggerCode();
void serverCode();
void childCode(tcpsock_t* server, tcpsock_t* client);

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

    splist_t* lijst = spl_create();

    close(pipefd[0]);

    write(pipefd[1], "Logger started!\n", strlen("Logger started!\n"));

    tcpsock_t *server, *client;

    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);

    printf("Server is started\n");
    write(pipefd[1], "Server started!\n", strlen("Server started!\n"));

    int loop = 0;
    while (running == 1 && loop < 10){
        loop++;

        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE);
        printf("Incoming client connection\n");
        
        int childProcess = fork();
        
        if (childProcess == 0)
            childCode(server, client);
        else{
            splist_node_t* ref = spl_get_first_reference(lijst);
            if (ref != NULL){
                while(ref != NULL){
                    int id = *(spl_get_element_at_reference(lijst, ref));
                    getpgid(id) == -1 ? spl_remove_at_reference(lijst, ref) : 0;
                    ref = spl_get_next_reference(lijst, ref);
                }
            }
            spl_insert_at_reference(lijst, &childProcess, NULL);
            char text[MAXTEXT];
            sprintf(text, "Process created, pid: %d\n", childProcess);
            write(pipefd[1], text, strlen(text));
        }
    }
    write(pipefd[1], "Closing down\n", strlen("Closing down\n"));
    close(pipefd[1]);
    //kill all clients
    if (spl_size(lijst) > 0){
        splist_node_t* ref = spl_get_first_reference(lijst);
        while (ref != NULL){
            int id = *(spl_get_element_at_reference(lijst, ref));
            kill(id, SIGKILL);
            ref = spl_get_next_reference(lijst, ref);
        }
    }
    spl_free(&lijst);
    wait(NULL);
    exit(EXIT_SUCCESS);
}

void childCode(tcpsock_t* server, tcpsock_t* client){
    int bytes, result;
    char data[BUFFER_MAX];
    int msgCount = 0;
    do {
        msgCount++;
        bytes = BUFFER_MAX;
        result = tcp_receive(client, (void *) &data, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("Received : %s\n", data);
            bytes = strlen(data);
            bytes = bytes > BUFFER_MAX ? BUFFER_MAX : bytes;
            write(pipefd[1], data, bytes);
            write(pipefd[1], "\n", strlen("\n"));
            result = tcp_send(client,(void *) &data, &bytes);
        }
    } while (result == TCP_NO_ERROR && msgCount < 10);
    if (result == TCP_CONNECTION_CLOSED){
        printf("Peer has closed connection\n");
        write(pipefd[1], "Peer has closed connection\n", strlen("Peer has closed connection\n"));
    } else{
        printf("Error occured on connection to peer\n");
        write(pipefd[1], "Error occured on connection to peer\n", strlen("Error occured on connection to peer\n"));
    }
    tcp_close(&client);

    _exit(EXIT_SUCCESS);

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