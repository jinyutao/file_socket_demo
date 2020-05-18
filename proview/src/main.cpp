#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "proc_agv.h"

struct globalArgs_t globalArgs =
{
    .is_daemon = 0,
    .is_svr = -1,
    .sock_fd = 0,
    .debug = 0
};
typedef struct
{
    pthread_t m_pthread;
    void* (*pFun)(void*);
}workthread_t;

static int stat_for_daemon();
static int main_process();
void* process_run(void*);
int client_run(int fd_remote);
int main(int agvc, char* agvr[])
{
    if(proc_arg(agvc, agvr) == 0)
    {
        return EXIT_SUCCESS;
    }

    if(globalArgs.is_daemon)
    {
        return stat_for_daemon();
    }
    if(!globalArgs.is_svr)
    {
        return client_run(globalArgs.sock_fd);
    }
    return main_process();
}

static int stat_for_daemon()
{
    printf("Start daemon\n");
    pid_t fpid; //fpid表示fork函数返回的值
    fpid = fork();
    if (fpid < 0)
    {
        printf("error in fork!");
        return EXIT_FAILURE;
    }
    else if (fpid == 0) {
        printf("Child process is start/n");
        return main_process();
    }
    else {
        printf("Parent process will exit../n");
        return EXIT_SUCCESS;
    }
}
static int main_process()
{
    pthread_t pthread_hd;

    pthread_create(&pthread_hd, NULL, process_run, NULL);
    pthread_join(pthread_hd, NULL);


    return EXIT_SUCCESS;
}