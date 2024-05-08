/**
 * @author Xander Verberckt
 */

//TODO: ADD MUTEX in insert

#include <stdlib.h> 
#include <string.h>
#include "sbuffer.h"
#include <pthread.h>
#define AMOUNT_READERS 2

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    char data[MESSAGE_MAX];         /**< a string with MESSAGE_MAX length */
    int read_by[AMOUNT_READERS];
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */

    pthread_mutex_t mutex;      //mutex
    pthread_mutexattr_t attr;   //mutex attribute
};

int sbuffer_init(sbuffer_t ** buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;

    //init the mutex
    if (pthread_mutexattr_init(&(*buffer)->attr) == -1) return SBUFFER_FAILURE;
    if (pthread_mutex_init(&(*buffer)->mutex, &(*buffer)->attr) == -1) return SBUFFER_FAILURE;

    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t ** buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    free(*buffer);
    *buffer = NULL;
    pthread_mutex_destroy(&(*buffer)->mutex);
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t * buffer, char * data, int who_am_i) {   //TODO: check if the both have read
    
    //check if both subtreads have read!!

    sbuffer_node_t *dummy;
    pthread_mutex_lock(&(buffer->mutex));
    if (buffer == NULL) {
        pthread_mutex_unlock(&(buffer->mutex));
        return SBUFFER_FAILURE;
    } 
    if (buffer->head == NULL){
        pthread_mutex_unlock(&(buffer->mutex));
        return SBUFFER_NO_DATA;
    } 

    dummy = buffer->head;

    while(dummy->read_by[who_am_i] == 1) {
        dummy = dummy->next;
        if (dummy == NULL) {
            pthread_mutex_unlock(&(buffer->mutex));
            return SBUFFER_NO_DATA;
        }   
    }

    strcpy(data, dummy->data);
    dummy->read_by[who_am_i] = 1;

    dummy = buffer->head;

    int i = 0;
    for (int j = 0; j < AMOUNT_READERS; j++)
        i += buffer->head->read_by[j];

    if (i == AMOUNT_READERS) {
        if (buffer->head == buffer->tail) // buffer has only one node
        {
            buffer->head = buffer->tail = NULL;
        } else  // buffer has many nodes empty
        {
            buffer->head = buffer->head->next;
        }
        free(dummy);
    }

    pthread_mutex_unlock(&(buffer->mutex));

    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t * buffer, char * data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;

    // --- from here critical section I think.. ---
    pthread_mutex_lock(&(buffer->mutex));
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) {
        pthread_mutex_unlock(&(buffer->mutex));
        return SBUFFER_NO_DATA;
    }

    strcpy(dummy->data, data);
    dummy->next = NULL;
    
    for (int i = 0; i < AMOUNT_READERS; i++)
        dummy->read_by[i] = 0;

    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    pthread_mutex_unlock(&(buffer->mutex));
    return SBUFFER_SUCCESS;
}
