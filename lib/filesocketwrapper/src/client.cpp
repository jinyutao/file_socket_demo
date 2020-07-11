#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <cstddef>

#include "fs_constants.h"

static pthread_t g_recv_run_thread = 0;
static int tcu_sf_disconn_l(int handle);
static int comp_key(const void * key, const void * tag);

static st_conn_mana g_ConnMana=
{
    tcu_sf_disconn_l,
    {NULL,0,0,0},
    PTHREAD_MUTEX_INITIALIZER,
    0
};

int tcu_sf_connect(const char* sock_file_name, notify_cb fun)
{
    printf("%s start ... \n",__FUNCTION__);
    int ret;
    if(fun == NULL || sock_file_name == NULL)
    {
        return -1;
    }
    // 启动线程
    if(g_recv_run_thread == 0)
    {
        if (DEBUG) printf("%s pthread_create() ... \n",__FUNCTION__);
        init_arr(&g_ConnMana.connect_info,sizeof(st_conn_info));
        //ret = pthread_create(&g_recv_run_thread, NULL, _run_client_recv_fun, NULL);
        ret = create_recv_thread(&g_recv_run_thread, &g_ConnMana);
        if(ret < 0)
        {
            return -errno;
        }
    }
    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    server_address.sun_path[0]='\0';
    strcpy(server_address.sun_path + 1, sock_file_name);

    int server_sockfd=socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(connect(server_sockfd,(struct sockaddr *)&server_address,
        offsetof(struct sockaddr_un, sun_path) + strlen(sock_file_name) + 1) < 0){
        printf("connect socket failed errno=%d [%s]\n", errno, strerror(errno));
        close(server_sockfd);
        return -errno;
    }
    if (DEBUG) printf("%s send hell message\n",__FUNCTION__);
    // 发送 握手报文，
    st_buff buff = {sizeof(HELLO_STRING),(byte*)HELLO_STRING};
    ret = tcu_sf_send(server_sockfd, &buff);
    if (DEBUG) printf("%s send hell message tcu_sf_send() ret=%d\n",__FUNCTION__,ret);
    if(ret < 0)
    {
        printf("%s tcu_sf_send() error:%d \n",__FUNCTION__,ret);
        close(server_sockfd);
        return ret;
    }
    // 等待  握手报文的应答
    size_t len;
    ret = read_s(server_sockfd, &len, MESSAGE_HEAD_LEN);
    if(ret < 0)
    {
        printf("%s read_s() error:%d \n",__FUNCTION__,ret);
        close(server_sockfd);
        return ret;
    }
    char temp[256]="\0";
    ret = read_s(server_sockfd,temp, len);
    if (DEBUG) printf("%s read_s()ret:%d %s \n",__FUNCTION__,ret,temp);

    // 加入队列
    st_conn_info info={
        server_sockfd,
        {0,NULL},
        0,0,0,0,
        fun
    };
    pthread_mutex_lock(&g_ConnMana.mutex_array_lock);
    append_arr(&g_ConnMana.connect_info, &info);
    pthread_mutex_unlock(&g_ConnMana.mutex_array_lock);
    write(g_ConnMana.pipeHandle,"add",3);
    return server_sockfd;
}

int tcu_sf_disconn(int handle)
{
    pthread_mutex_lock(&g_ConnMana.mutex_array_lock);
    int ret = 0;
    int i = find(&g_ConnMana.connect_info,&handle,comp_key);
    if(i>=0)
    {
        st_conn_info* p =
            (st_conn_info*)(g_ConnMana.connect_info.pBase+(g_ConnMana.connect_info.elm_size * i));
        if(p->handle != handle)
        {
            printf("%s:%d %s\n",__FILE__,__LINE__,__FUNCTION__);
            exit(-1);
        }
        p->error = -1;
        write(g_ConnMana.pipeHandle,"bye",3);
    }
    else
    {
        ret = -EBADF;
    }
    pthread_mutex_unlock(&g_ConnMana.mutex_array_lock);
    return ret;
}
int tcu_sf_disconn_l(int handle)
{
    int i = find(&g_ConnMana.connect_info,&handle,comp_key);
    if(i<0)
        return -EBADF;
    // 关闭
    close(handle);
    delete_arr(&g_ConnMana.connect_info,i,NULL);
    if (DEBUG) printf("%s close(%d) clients_info.cnt=%d\n",__FUNCTION__,handle,g_ConnMana.connect_info.cnt);
    return 0;
}

static int comp_key(const void * key, const void * tag)
{
    const int handle = *((int*)key);
    const st_conn_info* pTag = (st_conn_info*)tag;
    return handle - pTag->handle;
}