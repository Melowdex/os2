#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static void demo_SIGINT_handler(int sig);

static volatile int running = 1;

int main(int argc, char *argv[]) {
    struct sigaction sa;
    sa.sa_handler = demo_SIGINT_handler;
    sigaction(SIGINT, &sa, NULL);

    while (running == 1) {
        printf("still running....\n");
        sleep(2);
    }

    printf("loop is finished...\n");
    return 0;
}

static void demo_SIGINT_handler(int sig) {
    printf("signal received by application\n");
    running = 0;
}