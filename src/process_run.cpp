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

#include "proc_agv.h"
#define DEBUG (0)
#define MAX_CONNECTION_NUMBER 1
long getElapsedRealtime();
ssize_t write(int fd, void const* vaddr, size_t size)
{
    ssize_t err, len;
    do {
        len = ::send(fd, vaddr, size, MSG_DONTWAIT | MSG_NOSIGNAL);
        // cannot return less than size, since we're using SOCK_SEQPACKET
        err = len < 0 ? errno : 0;
    } while (err == EINTR);
    return err == 0 ? len : -err;
}
ssize_t read_s(int fd, void* vaddr, size_t size)
{
    ssize_t err, len;
    do {
        len = ::recv(fd, vaddr, size, 0);
        err = len < 0 ? errno : 0;
    } while (err == EINTR);
    if (err == EAGAIN || err == EWOULDBLOCK) {
        // EAGAIN means that we have non-blocking I/O but there was
        // no data to be read. Nothing the client should care about.
        return 0;
    }
    return err == 0 ? len : -err;
}
void * process_run(void * p )
{
    int ret;

    char message[256];
    int server_sockfd;
    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, FILE_SOCKET_NAME);
    server_sockfd=socket(AF_UNIX, SOCK_SEQPACKET, 0);
    printf("server_sockfd=%d\n",server_sockfd);
    do
    {
        errno =0;
        ret = bind(server_sockfd, (struct sockaddr*)&server_address, sizeof(server_address));
        ret = ret==0?0:-errno;
        printf("bind()=%d errno=%d[%s]\n",ret,errno, strerror(errno));
        if(ret && ret==(-EADDRINUSE))
        {
            printf("unlink()=%d errno=%d\n",unlink(FILE_SOCKET_NAME),errno);
        }
    } while (ret);

    // ret = bind(server_sockfd, (struct sockaddr*)&server_address, sizeof(server_address));
    // printf("bind()=%d errno=%d\n",ret,errno);

    if(listen(server_sockfd,1) < 0){
        printf("listen failed! errno=%d[%s]\n",errno, strerror(errno));
        return NULL;
    }
    int i=0;
    while(1)
    {
        if (DEBUG) printf("\nwait accept()... %d \n", i++);
        int new_fd = accept(server_sockfd,NULL,NULL);
        if(new_fd < 0){
            printf("accept failed errno=%d[%s]\n",errno, strerror(errno));
            return NULL;
        }
        long ts=getElapsedRealtime();
        while(1)
        {
            if (DEBUG) printf("wait recvfrom()...\n");
            ssize_t getdatalen = read(
                    new_fd,
                    message,
                    sizeof(message));
            if(getdatalen<=0)
            {
                if (DEBUG) printf("recv()=%ld errno=%d [%s]\n",getdatalen, errno, strerror(errno));
                break;
            }
            if (DEBUG) printf("recv()=%ld %s\n",getdatalen, message);
            int r = write(new_fd, message, getdatalen);
            if (DEBUG) printf("sendto() ret=%d  errno=%d [%s]\n",r, errno, strerror(errno));
        }
        close(new_fd);
        if (DEBUG) printf("connect time %ld ms\n\n",getElapsedRealtime()-ts);
    }
    close(server_sockfd);
    return NULL;
}

