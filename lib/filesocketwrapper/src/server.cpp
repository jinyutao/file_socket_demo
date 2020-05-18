#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "com_func.h"

#include "fs_constants.h"
#include "array_opt.h"
#include "filesocketwrapper.h"

int server_sockfd = -1;
notify_cb g_fun;

static int comp_key(const void * key, const void * tag);
static int tcu_sf_close_client_l(int client_handle);

static pthread_t g_recv_run_thread = 0;

static void* _run_svr_accept_fun(void * p);
static pthread_t g_accept_run_thread = 0;

static st_conn_mana g_ConnMana=
{
    tcu_sf_close_client_l,
    {NULL,0,0,0},
    PTHREAD_MUTEX_INITIALIZER,
    0
};
int tcu_sf_listen(const char* sock_file_name, int max_listen, notify_cb fun)
{
    int ret;
    if(server_sockfd >= 0 || fun == NULL)
        return -1;
    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, sock_file_name);
    server_sockfd=socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (DEBUG) printf("server_sockfd=%d\n",server_sockfd);
    do
    {
        errno =0;
        ret = bind(server_sockfd, (struct sockaddr*)&server_address, sizeof(server_address));
        ret = ret==0?0:-errno;
        if (DEBUG) printf("bind()=%d errno=%d[%s]\n",ret,errno, strerror(errno));
        if(ret && ret==(-EADDRINUSE))
        {
            printf("unlink()=%d errno=%d\n",unlink(sock_file_name),errno);
        }
    } while (ret);

    if(listen(server_sockfd,max_listen) < 0){
        printf("listen failed! errno=%d[%s]\n",errno, strerror(errno));
        return -errno;
    }
    g_fun = fun;

    // 启动线程
    if(g_accept_run_thread == 0)
    {
        ret = pthread_create(&g_accept_run_thread, NULL, _run_svr_accept_fun, NULL);
        if(ret < 0)
        {
            return -errno;
        }
    }
    return 0;
}
int tcu_sf_close_client(int client_handle)
{
    pthread_mutex_lock(&g_ConnMana.mutex_array_lock);
    int ret = 0;
    int i = find(&g_ConnMana.connect_info,&client_handle,comp_key);
    if(i>=0)
    {
        st_conn_info* p =
            (st_conn_info*)(g_ConnMana.connect_info.pBase+(g_ConnMana.connect_info.elm_size * i));
        if(p->handle != client_handle)
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
int tcu_sf_close_client_l(int client_handle)
{
    int i = find(&g_ConnMana.connect_info,&client_handle,comp_key);
    if (DEBUG) printf("%s client_handle find i=%d ... \n",__FUNCTION__,i);
    if(i<0)
    {
        printf("%s client_handle not find ... \n",__FUNCTION__);
        return -EBADF;
    }
    // 关闭
    close(client_handle);
    // 删除队列
    delete_arr(&g_ConnMana.connect_info,i,NULL);
    if (DEBUG) printf("%s close(client_handle) connect_info.cnt=%d\n",__FUNCTION__,g_ConnMana.connect_info.cnt);
    return 0;
}
int tcu_sf_close_server()
{
    // 停止线程
    close(server_sockfd);
    server_sockfd = -1;
    return 0;
}

static void* _run_svr_accept_fun(void * p)
{
    printf("%s start ... \n",__FUNCTION__);
    int ret;
    fd_set sockfd_set;
    struct timeval timeout_val;
    if(g_ConnMana.connect_info.pBase==NULL)
    {
        init_arr(&g_ConnMana.connect_info,sizeof(st_conn_info));
    }
    // 启动recv线程
    while (g_recv_run_thread == 0)
    {
        // ret = pthread_create(&g_recv_run_thread, NULL, _run_svr_recv_fun, NULL);
        ret = create_recv_thread(&g_recv_run_thread, &g_ConnMana);
        if(ret < 0)
        {
            sleep(5);
            continue;
        }
    }
    while(1)
    {
        int new_fd = accept(server_sockfd,NULL,NULL);
        if(new_fd < 0){
            printf("accept failed errno=%d[%s]\n",errno, strerror(errno));
            sleep(5);
            tcu_sf_close_server();
            return NULL;
        }
        if (DEBUG) printf("%s accept() ret=%d  \n",__FUNCTION__,new_fd);

        // 客户端必须在50毫秒内发送握手消息
        FD_ZERO(&sockfd_set);
        FD_SET(new_fd, &sockfd_set);
        timeout_val.tv_sec = 0;
        timeout_val.tv_usec = 50*1000*1000;
        int retval = select(new_fd + 1, &sockfd_set, NULL, NULL, &timeout_val);
        if(retval<=0 || !FD_ISSET(new_fd, &sockfd_set))
        {
            close(new_fd);
            continue;
        }
        size_t len;
        ret = read_s(new_fd,&len,MESSAGE_HEAD_LEN);
        if(ret < 0 || len!=sizeof(HELLO_STRING))
        {
            close(new_fd);
            continue;
        }

        char hell_buff[128];
        ret = read_s(new_fd,hell_buff,len);
        if(ret < 0)
        {
            close(new_fd);
            continue;
        }
        if(strncmp(hell_buff,HELLO_STRING,len))
        {
            printf("%s  read_s() read_s msg_head err %s \n",__FUNCTION__,hell_buff);
            close(new_fd);
            continue;
        }
        st_buff buff = {sizeof(HELLO_STRING),(byte*)HELLO_STRING};
        ret = tcu_sf_send(new_fd, &buff);
        // 加入队列
        st_conn_info info={
            new_fd,
            {0,NULL},
            0,0,0,0,
            g_fun
        };
        pthread_mutex_lock(&g_ConnMana.mutex_array_lock);
        append_arr(&g_ConnMana.connect_info, &info);
        pthread_mutex_unlock(&g_ConnMana.mutex_array_lock);
        write(g_ConnMana.pipeHandle,"add",3);
        g_fun(new_fd, event_type::tcu_sf_accept,0, NULL);
    }
}
static int comp_key(const void * key, const void * tag)
{
    const int handle = *((int*)key);
    if (DEBUG) printf( "%s() handle:%d\n",__FUNCTION__,handle);
    const st_conn_info* pTag = (st_conn_info*)tag;
    return handle - pTag->handle;
}