#include <stdio.h>
#include <pthread.h>
#include "sbuffer.h"
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <string.h>
#define MESSAGE_MAX 512

struct thread_info {    /* Used as argument to thread_start() */
    pthread_t thread_id;        /* ID returned by pthread_create() */
    int       thread_num;       /* Application-defined thread # */
    char     *argv_string;      /* From command-line argument */
};

int done;

void* freq_count(void* arg) {   //freq_count is 0

    sbuffer_t **buffer = (sbuffer_t **) arg;
    printf("freq_count\n");

    int* char_count = calloc(26, sizeof(int));
    int ret;

    while(done == 0 || ret != SBUFFER_NO_DATA){
        char *data = malloc(MESSAGE_MAX);

        ret = sbuffer_remove(*buffer, data, 0); 

        if (ret == SBUFFER_SUCCESS) {
            for (int i = 0; i < MESSAGE_MAX; i++) {
                if (data[i] == '\0') {
                    break;
                }
                if (isalpha(data[i])){
                    char dummy = tolower(data[i]);
                    char_count[dummy - 'a']++;
                }
            }
        }
        else if (ret == SBUFFER_FAILURE) {
            printf("errorrrr\n");
        }

        free(data);
    }

    for (int i = 0; i < 26; i++) {
        printf("%c: %d\n", 'a' + i, char_count[i]);
    }

    pthread_exit(NULL);
}

void* digest(void* arg) {   //digest is 1
    sbuffer_t **buffer = (sbuffer_t**) arg;

    FILE *file = fopen("sha.txt", "w");
    
    printf("digest\n");

    int ret;

    while(done == 0 || ret != SBUFFER_NO_DATA){
        char *data = malloc(MESSAGE_MAX);

        ret = sbuffer_remove(*buffer, data, 1); 

        if (ret == SBUFFER_SUCCESS) {
            printf("data: %s\n", data);
            unsigned char sha[MESSAGE_MAX];
            SHA1((unsigned char*) data, strlen(data), sha);
            printf("SHA1: %s\n", sha);
            fprintf(file, "%s\n", sha);
        }
        else if (ret == SBUFFER_FAILURE) {
            printf("errorrrr\n");
        }

        free(data);
    }

    fclose(file);
    pthread_exit(NULL);
}

void t_reader(sbuffer_t **buffer) {
    printf("t_reader\n");
    FILE *f = fopen("input.txt", "r");
    char *line = NULL;
    size_t len = 0;
    if (f == NULL) {
        fprintf(stderr, "NI GOE!! file open gefaald..\n");
        return;
    }
    while(getline(&line, &len, f) != -1){
        sbuffer_insert(*buffer, line);
    }
    fclose(f);
    sleep(1);
    done = 1;
}



int main() {

    int s, t;
    pthread_t freq_count_id, digest_id;
    sbuffer_t *buffer;
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "NI GOE!! buffer init gefaald..\n");
        return -1;
    }
    done = 0;
    s = pthread_create(&freq_count_id, NULL, freq_count, &buffer);
    t = pthread_create(&digest_id, NULL, digest, &buffer);

    t_reader(&buffer);

    pthread_join(t, NULL);
    pthread_join(s, NULL);

    printf("KHEB GEDAAAAAANNN\n");


    return 0;
}