#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


int main(){

    int pipefd[2];
    char buf[50];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }


    int process = fork();

    if (process == 0){  //child code

        close(pipefd[1]);
        FILE* f = fopen("log.txt", "a");
        
        while (read(pipefd[0], buf, 50) > 0)
            fprintf(f, "%s", buf);
        
        fclose(f);
        close(pipefd[0]);
        _exit(EXIT_SUCCESS);

    }else{              //parent code

        close(pipefd[0]);
        write(pipefd[1], "Hello world! \n", strlen("Hello world!"));
        close(pipefd[1]);
        wait(NULL);
        exit(EXIT_SUCCESS);

    }

    return 0;
}