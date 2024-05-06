#include <stdio.h>
#include <pthread.h>
#include "sbuffer.h"
#include <ctype.h>
#include <stdlib.h>
#define MESSAGE_MAX 512

struct thread_info {    /* Used as argument to thread_start() */
    pthread_t thread_id;        /* ID returned by pthread_create() */
    int       thread_num;       /* Application-defined thread # */
    char     *argv_string;      /* From command-line argument */
};

void freq_count(sbuffer_t **buffer) {   //freq_count is 0
    printf("freq_count\n");

    int char_count[26] = {0};

    while(1){
        char *data = malloc(MESSAGE_MAX);

        int ret = sbuffer_remove(*buffer, data, 0); 

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
        else if (ret == SBUFFER_NO_DATA) {
            printf("data gone\n");
            sleep(1);
        }

        free(data);
    }
}

void digest(sbuffer_t **buffer) {   //digest is 1
    printf("digest\n");
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
}



int main() {

    int s, t;
    pthread_t freq_count_id, digest_id;
    sbuffer_t *buffer;
    if (sbuffer_init(&buffer) != SBUFFER_SUCCESS) {
        fprintf(stderr, "NI GOE!! buffer init gefaald..\n");
        return -1;
    }
    s = pthread_create(&freq_count_id, NULL, freq_count, &buffer);
    t = pthread_create(&digest_id, NULL, digest, &buffer);

    t_reader(&buffer);



    return 0;
}