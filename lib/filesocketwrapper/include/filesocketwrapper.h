#ifndef FILESOCKET_WRAPPER_H
#define FILESOCKET_WRAPPER_H

#include <sys/types.h>
#include "com_def.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// 存储报文的BUFF
typedef struct
{
    ssize_t len;    // 报文的长度
    byte* data;     // data指向的内存由调用者创建，由调用者销毁
}st_buff;

// 事件类型
typedef enum
{
    tcu_sf_accept,  // 服务端成功收到来自客户端的连接请求，并成功握手
    tcu_sf_recv,    // 收到来自远端的数据
    tcu_sf_closed   // 连接被关闭
}event_type;

// 通知事件的回调函数
// @client_handle   发生事件的socket句柄
// @event           发生事件的类型
//              如果类型是【tcu_sf_closed】可以参考参数error，确定原因
// @error             0:没有错误
//                   -1:主动关闭 或
//                      远端被关闭了
//            其他<0的数：出错时的错误代码
// @st_buff   接收到的报文
typedef void (*notify_cb)(int client_handle,event_type event,int error,st_buff* recv_data);

// 客户端函数

// 连接服务端
// @sock_file_name  服务端的socket file文件名
// @fun             通知事件的回调函数指针
// @ret             >=0 连接成功，返回本地socket句柄
//                   <0 失败，返回-errno
int tcu_sf_connect(const char* sock_file_name, notify_cb fun);
// 关闭连接
// @handle      tcu_sf_connect函数返回的socket句柄
// @ret         =0 成功
//              <0 失败，返回-errno
int tcu_sf_disconn(int handle);

// 服务端函数
// 启动服务端开始侦听
// @sock_file_name  服务端的socket file文件名
// @max_listen      最大连接数
// @fun             通知事件的回调函数指针
int tcu_sf_listen(const char* sock_file_name, int max_listen, notify_cb fun);
// 主动关闭（断开）来自客户端的连接
// @client_handle   客户端连接的socket句柄
//                    通过tcu_sf_listen的回调函数，
//                    在事件为tcu_sf_accept时，传出的socket句柄
int tcu_sf_close_client(int client_handle);
// 关闭服务端的侦听，停止服务
int tcu_sf_close_server();

// 服务端和客户端函数
// 发送数据
// @handle       连接的socket句柄
// @buff        待发送的报文。
ssize_t tcu_sf_send(int handle, st_buff* buff);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // FILESOCKET_WRAPPER_H