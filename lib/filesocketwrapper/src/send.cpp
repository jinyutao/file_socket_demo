#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>

#include "filesocketwrapper.h"

static ssize_t write_s(int fd, void const* vaddr, size_t size);

ssize_t tcu_sf_send(int handle, st_buff* buff)
{
    int ret = write_s(handle, &buff->len, sizeof(size_t));
    if(ret<0) return ret;
    return write_s(handle, buff->data, buff->len);
}

static ssize_t write_s(int fd, void const* vaddr, size_t size)
{
    ssize_t err, len;
    do {
        len = ::send(fd, vaddr, size, MSG_DONTWAIT | MSG_NOSIGNAL);
        // cannot return less than size, since we're using SOCK_SEQPACKET
        err = len < 0 ? errno : 0;
    } while (err == EINTR);
    return err == 0 ? len : -err;
}
