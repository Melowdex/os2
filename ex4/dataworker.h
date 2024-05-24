#include <stdio.h>
#include <pthread.h>
#include "sbuffer.h"

extern volatile int running;
extern int pipefd_toDataWorker[2];

int start_data_worker();
