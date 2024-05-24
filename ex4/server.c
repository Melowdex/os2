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
#include "dataworker.h"

#define PORT 3000
#define MAXBUFFER 150
#define MAXTEXT 60

void loggerCode();
void serverCode(int processLogger);
void childCode(tcpsock_t* server, tcpsock_t* client);

static void SIGINT_handler(int sig);
static void SIGUSR1_handler(int sig);

volatile int running = 1;
static volatile int loggerRunning = 1;
int pipefd[2];
int pipefd_toDataWorker[2];
char buf[MAXBUFFER];

int main(){

    struct sigaction sa2;
    sa2.sa_handler = SIGUSR1_handler;
    sigaction(SIGUSR1, &sa2, NULL);

    struct sigaction sa;
    sa.sa_handler = SIGINT_handler;
    sigaction(SIGINT, &sa, NULL);

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int processLogger = fork();

    if (processLogger == 0){
        loggerCode();
    } else{
        serverCode(processLogger);
    }
    return 0;
}

void serverCode(int processLogger){

    if (pipe(pipefd_toDataWorker) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int dataworker = fork();
    if (dataworker == 0){
        start_data_worker();
    } else{
    
        
        splist_t* lijst = spl_create();

        close(pipefd[0]);
        close(pipefd_toDataWorker[0]);

        write(pipefd[1], "Logger started!\n", strlen("Logger started!\n")+1);

        tcpsock_t *server, *client;

        if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);

        printf("Server is started\n");
        write(pipefd[1], "Server started!\n", strlen("Server started!\n")+1);

        int loop = 0;
        while (running == 1 && loop < 10){
            loop++;

            printf("Waiting for incoming client connection\n");

            if (tcp_wait_for_connection(server, &client) == TCP_NO_ERROR){
                printf("Incoming client connection\n");

                int childProcess = fork();

                if (childProcess == 0){
                    childCode(server, client);
                } else{
                    splist_node_t* ref = spl_get_first_reference(lijst);
                    if (ref != NULL){
                        while(ref != NULL){
                            int id = *(spl_get_element_at_reference(lijst, ref));
                            getpgid(id) == -1 ? spl_remove_at_reference(lijst, ref) : 0;
                            ref = spl_get_next_reference(lijst, ref);
                        }
                    }
                    spl_insert_at_reference(lijst, &childProcess, NULL);
                    printf("Process created, pid: %d\n", childProcess);
                    printf("Number of clients: %d\n", spl_size(lijst));
                    char text[MAXTEXT];
                    sprintf(text, "Process created, pid: %d\n", childProcess);
                    write(pipefd[1], text, strlen(text)+1);
                }
            }
        }
        printf("dag dag\n");
        write(pipefd[1], "Closing down\n", strlen("Closing down\n")+1);
        //kill all clients + logger
        sleep(0.2);
        if (spl_size(lijst) > 0){
            splist_node_t* ref = spl_get_first_reference(lijst);
            while (ref != NULL){
                int id = *(spl_get_element_at_reference(lijst, ref));
                printf("Killing process %d\n", id);
                kill(id, SIGUSR1);
                sleep(0.2);
                ref = spl_get_next_reference(lijst, ref);
            }
        } 
        spl_free(&lijst);
        close(pipefd[1]);
        close(pipefd_toDataWorker[1]);

        kill(processLogger, SIGUSR1);

        wait(NULL);
        sleep(1);
        exit(EXIT_SUCCESS);
    }
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
            write(pipefd[1], "\n", strlen("\n")+1);

            write(pipefd_toDataWorker[1], data, bytes);
            write(pipefd_toDataWorker[1], "\n", strlen("\n")+1);

            result = tcp_send(client,(void *) &data, &bytes);
        }
    } while (result == TCP_NO_ERROR && msgCount < 10 && running == 1);
    if (result == TCP_CONNECTION_CLOSED){
        printf("Peer has closed connection\n");
        write(pipefd[1], "Peer has closed connection\n", strlen("Peer has closed connection\n")+1);
    } else{
        printf("Error occured on connection to peer\n");
        write(pipefd[1], "Error occured on connection to peer\n", strlen("Error occured on connection to peer\n")+1);
    }
    tcp_close(&client);
    _exit(EXIT_SUCCESS);

}

void loggerCode(){
    int i = 0;
    int rres;
    close(pipefd[1]);
    FILE* f = fopen("logger.txt", "w");
    
    do {
        rres = read(pipefd[0], buf, MAXBUFFER);
        if (rres > 0){  
            //printf("%d) %s", i, buf);
            fprintf(f, "%d) %s", i, buf);
            fflush(f);
            ++i;
        }
        sleep(0.1);
    } while (running == 1);
    
    fclose(f);
    close(pipefd[0]);
    printf("Logger closed\n");
    _exit(EXIT_SUCCESS);
}

static void SIGINT_handler(int sig) {
    printf("signal received by application\n");
    running = 0;
}

static void SIGUSR1_handler(int sig) {
    printf("signal received by application\n");
    running = 0;
}
