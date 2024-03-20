#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAXBUFFER 100


int main(){

    int pipefd[2];
    char buf[MAXBUFFER];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }


    int process = fork();

    if (process == 0){  //child code

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

    }else{              //parent code

        close(pipefd[0]);
        for (int i = 0; i<5; i++){
            write(pipefd[1], "Hello world!\n", strlen("Hello world!\n"));
            sleep(1);
        }
        close(pipefd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);

    }

    return 0;
}