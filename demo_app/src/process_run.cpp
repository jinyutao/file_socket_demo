#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "filesocketwrapper.h"
#include "constant.h"

bool stop = false;

static void listen_cb_fun(int client_handle, event_type event, int error, st_buff* recv_data);

void * process_run(void * p )
{
    int ret;
    int server_handle;
    ret = tcu_sf_listen(FILE_SOCKET_NAME, 3, listen_cb_fun);
    printf("tcu_sf_listen() ret=%d",ret);
    server_handle = ret;
    while(!stop)
    {
        sleep(3);
    }
    tcu_sf_close_server();

    return NULL;
}
void listen_cb_fun(int client_handle,event_type event, int error, st_buff* recv_data)
{
    if(event == event_type::tcu_sf_recv)
    {
        st_buff b;
        b.len = recv_data->len;
        b.data = (byte*)malloc(recv_data->len);
        memcpy(b.data ,recv_data->data, b.len);
        int ret = tcu_sf_send(client_handle,&b);
        // printf("%s() tcu_sf_send():ret=%d(%s) data:%s \n",__FUNCTION__,ret,ret<0?strerror(-ret):"",b.data);

        free(b.data);
    }
    else
    {
        printf("%s() client_handle:%d event:%d \n",__FUNCTION__,client_handle,event);
    }
}
