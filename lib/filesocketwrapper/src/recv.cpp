#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>

#include "com_func.h"
#include "fs_constants.h"

static void* _run_recv_fun(void * p);

int create_recv_thread(pthread_t* thread_handle, st_conn_mana* pRecv)
{
    pthread_attr_t attr;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    int ret = pthread_create(thread_handle, &attr, _run_recv_fun, pRecv);
    return ret<0 ? -errno : ret;
}

static void* _run_recv_fun(void * p)
{
    st_conn_mana* pMan = (st_conn_mana*)p;
    fd_set sockfd_set;
    int ret;
    struct timeval timeout_val;
    int pipehandle[2];
    pipe(pipehandle);
    pMan->pipeHandle = pipehandle[1];

    while(1)
    {
        FD_ZERO(&sockfd_set);
        FD_SET(pipehandle[0], &sockfd_set);
        int maxHandle = pipehandle[0];
        pthread_mutex_lock(&pMan->mutex_array_lock);
        st_conn_info* pB = (st_conn_info*)pMan->connect_info.pBase;
        for(int i = 0; i<pMan->connect_info.cnt; i++,pB++)
        {
            if(pB->error == 0)
            {
                maxHandle = MAX(maxHandle, pB->handle);
                FD_SET(pB->handle, &sockfd_set);
                if (DEBUG) printf("%s add(%d) to sockfd_set\n",__FUNCTION__,pB->handle);
            }
        }
        pthread_mutex_unlock(&pMan->mutex_array_lock);

        timeout_val.tv_sec = SELECT_TIMEOUT_MILSEC / 1000;
        timeout_val.tv_usec = (SELECT_TIMEOUT_MILSEC - (timeout_val.tv_sec * 1000)) * 1000;
        int retval = select(maxHandle + 1, &sockfd_set, NULL, NULL, &timeout_val);
        if (retval < 0)
        {
            printf("%s select() ret=%d err:%d[%s]\n",__FUNCTION__, retval,errno,strerror(errno));
            break;
        }
        pthread_mutex_lock(&pMan->mutex_array_lock);
        pB = (st_conn_info*)pMan->connect_info.pBase;
        for(int i = 0; i<pMan->connect_info.cnt; i++,pB++)
        {
            if(FD_ISSET(pB->handle, &sockfd_set))
            {
                if (DEBUG) printf("%s find socket() handle:%d\n",__FUNCTION__, pB->handle);
                if(pB->len_byte_pos < MESSAGE_HEAD_LEN)
                {
                    byte tmp[MESSAGE_HEAD_LEN];
                    memcpy(tmp, &(pB->buff.len), MESSAGE_HEAD_LEN);
                    ret = read_s(pB->handle,
                        tmp + pB->len_byte_pos,
                        MESSAGE_HEAD_LEN - pB->len_byte_pos);
                    if (ret > 0)
                    {
                        memcpy(&(pB->buff.len),tmp,MESSAGE_HEAD_LEN);
                        pB->len_byte_pos += ret;
                        if(pB->len_byte_pos == MESSAGE_HEAD_LEN)
                        {
                            // ?? pB->buff.len = ntol(pB->buff.len);
                            if(pB->malloc_len > 0 && pB->buff.len > pB->malloc_len)
                            {
                                free(pB->buff.data);
                            }
                            pB->buff.data = (byte*)malloc(pB->buff.len);
                        }
                    }
                    else
                    {
                        if (DEBUG) printf("%s find read_s(%d) ret:%d\n",__FUNCTION__,
                           pB->handle, ret);
                        pB->error = (ret == 0) ? -1 : -errno;
                        continue;
                    }
                }
                if(pB->len_byte_pos == MESSAGE_HEAD_LEN)
                {
                    if (pB->read_bye_cnt < pB->buff.len)
                    {
                        ret = read_s(pB->handle,pB->buff.data + pB->read_bye_cnt, pB->buff.len - pB->read_bye_cnt);
                        if (ret > 0)
                        {
                            pB->read_bye_cnt += ret;
                            if(pB->read_bye_cnt == pB->buff.len)
                            {
                                pB->fun(pB->handle, event_type::tcu_sf_recv, 0, &pB->buff);
                                pB->len_byte_pos = 0;
                                pB->read_bye_cnt = 0;
                                pB->buff.len = 0;
                            }
                        }
                        else
                        {
                            if (DEBUG) printf("%s find read_s(%d) ret:%d\n",__FUNCTION__,
                                pB->handle, ret);
                            pB->error = (ret == 0) ? -1 : -errno;
                            continue;
                        }
                    }
                }
            }
        }
        // close error handle?
        pthread_mutex_unlock(&pMan->mutex_array_lock);
        if(FD_ISSET(pipehandle[0], &sockfd_set))
        {
            char tmp[10];
            memset(tmp,0,sizeof(tmp));
            read(pipehandle[0],tmp,10);
            if (DEBUG) printf("notify from pipe [%s]\n",tmp);
        }
        pthread_mutex_lock(&pMan->mutex_array_lock);
        for(int i = pMan->connect_info.cnt - 1; i >=0 ; i--)
        {
            pB = (st_conn_info*)(pMan->connect_info.pBase + pMan->connect_info.elm_size * i);
            if(pB->error!=0)
            {
                pB->fun(pB->handle,event_type::tcu_sf_closed, pB->error, NULL);
                if(pB->buff.data) {free(pB->buff.data);}
                pMan->disconn(pB->handle);
            }
        }
        pthread_mutex_unlock(&pMan->mutex_array_lock);
    }
    return NULL;
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