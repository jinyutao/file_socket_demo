#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

#include "proc_agv.h"
ssize_t write(int fd, void const* vaddr, size_t size);
ssize_t read_s(int fd, void* vaddr, size_t size);

long getElapsedRealtime();

int client_run(int fd_remote)
{
    int ret;
    struct timeval tv;
    char tmp[512];

    struct sockaddr_un server_address;
    socklen_t sin_len = sizeof(struct sockaddr);
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, FILE_SOCKET_NAME);
    long ts=getElapsedRealtime();
    for(int j =0; j<100;j++)
    {
    int server_sockfd=socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(connect(server_sockfd,(struct sockaddr *)&server_address,sizeof(server_address)) < 0){
        printf("connect socket failed errno=%d [%s]\n", errno, strerror(errno));
        return -1;
    }
for(int i=0;i<10;i++)
{
    size_t l;
    printf("-----for %d ------\n",i);

    memset(tmp,0,sizeof(tmp));
    l = sprintf(tmp,"[%d] ",i);

    gettimeofday(&tv, NULL);
    struct tm *ptm = gmtime(&tv.tv_sec);
    l += strftime(tmp+l,sizeof(tmp)-l,"%F %T",ptm);
    snprintf(tmp+l,sizeof(tmp) - l,".%06ld", tv.tv_usec);
    printf("client-send() %s\n",tmp);

    ret = write(server_sockfd, tmp,strlen(tmp));
    printf("client-sent() ret:%d \n",ret);

    memset(tmp,0,sizeof(tmp));
    ret = read_s(server_sockfd,tmp,sizeof(tmp));
    printf("client-recv()ret:%d %s\n------\n",ret,tmp);
}
    close(server_sockfd);
    }
    printf("run time: %ld ms\n", getElapsedRealtime() - ts);
    return EXIT_SUCCESS;
}


// static
long getElapsedRealtime()
{
    struct timespec now_sys_clock;
    clock_gettime(CLOCK_REALTIME, &now_sys_clock);
    return now_sys_clock.tv_sec * 1000 + now_sys_clock.tv_nsec / 1000000;
}