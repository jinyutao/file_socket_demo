#ifndef FS_CONSTANTS_H
#define FS_CONSTANTS_H

#include <unistd.h>
#include <pthread.h>
#include "filesocketwrapper.h"
#include "array_opt.h"

#define DEBUG (0)

#define HELLO_STRING ("5d4ff6f8-934d-11ea-9947-d7c2e77ec9d5")

#define SELECT_TIMEOUT_MILSEC (10*1000)
#define MESSAGE_HEAD_LEN (sizeof(size_t))

typedef struct
{
    int handle;
    st_buff buff;
    size_t len_byte_pos;
    int read_bye_cnt;
    int malloc_len;
    int error;
    notify_cb fun;
}st_conn_info;

typedef struct
{
    int (*disconn)(int handle);
    struct Arr connect_info;
    pthread_mutex_t mutex_array_lock;
    int pipeHandle;
}st_conn_mana;

int create_recv_thread(pthread_t* thread_handle, st_conn_mana* pRecv);
ssize_t read_s(int fd, void* vaddr, size_t size);

#endif //FS_CONSTANTS_H