#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <semaphore.h>

#include "filesocketwrapper.h"
#include "constant.h"

static void recv_cb_fun(int handle,event_type event, int error, st_buff* buff);
sem_t got_recv;
int got_recv_cnt;
#define TEST_SEND_MSG_CNT (100)
int client_run()
{
    sem_init(&got_recv, 0, 0);
    int ret;
    char tmp[512];
    struct timeval tv;
    long ts = getElapsedRealtime();
    for(int j =0; j<100;j++)
    {
        printf("====for %d ====\n",j);
        got_recv_cnt = 0;
        printf("%s tcu_sf_connect\n",__FUNCTION__);
        ret = tcu_sf_connect(FILE_SOCKET_NAME, recv_cb_fun);
        int handle = ret;
        printf("tcu_sf_connect() ret:%d\n",ret);
        if(handle<0) continue;
        for(int i=0;i<TEST_SEND_MSG_CNT;i++)
        {
            size_t l;

            memset(tmp,0,sizeof(tmp));
            l = sprintf(tmp,"[%d] [%5d]",j,i);

            gettimeofday(&tv, NULL);
            struct tm *ptm = gmtime(&tv.tv_sec);
            l += strftime(tmp+l,sizeof(tmp)-l,"%F %T",ptm);
            snprintf(tmp+l,sizeof(tmp) - l,".%06ld", tv.tv_usec);
            //printf("client-send() %s\n",tmp);
            st_buff b={
                (int)strlen(tmp),
                (byte*)tmp
            };
            tcu_sf_send(handle,&b);
        }
        sem_wait(&got_recv);
        tcu_sf_disconn(handle);
    }
    printf("run time: %ld ms\n", getElapsedRealtime() - ts);

    sem_destroy(&got_recv);
    return EXIT_SUCCESS;
}

static void recv_cb_fun(int handle,event_type event, int error, st_buff* buff)
{
    char tmp[512];
    struct timeval tv;
    switch(event)
    {
        case tcu_sf_recv:
        {
            gettimeofday(&tv, NULL);
            struct tm *ptm = gmtime(&tv.tv_sec);
            size_t l = strftime(tmp,sizeof(tmp),"%F %T",ptm);
            snprintf(tmp+l,sizeof(tmp) - l,".%06ld", tv.tv_usec);

            printf("%s() [%5d] \n         %s \n                     %s \n",
                __FUNCTION__,got_recv_cnt,buff->data,tmp);
            got_recv_cnt++;
            if(got_recv_cnt>=TEST_SEND_MSG_CNT)
            {
                sem_post(&got_recv);
            }
        }
        case tcu_sf_closed:
            break;
        default:
        {
            printf("%s() error:%d \n",__FUNCTION__,error);
            sem_post(&got_recv);
            exit(1);
        }
    }
}

// static
long getElapsedRealtime()
{
    struct timespec now_sys_clock;
    clock_gettime(CLOCK_REALTIME, &now_sys_clock);
    return now_sys_clock.tv_sec * 1000 + now_sys_clock.tv_nsec / 1000000;
}