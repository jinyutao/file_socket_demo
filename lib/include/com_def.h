#ifndef COM_DEF_H
#define COM_DEF_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef BOOL
#define BOOL char
#endif // BOOL

#ifndef BYTE
#define BYTE unsigned char
#endif // BYTE

#ifndef byte
#define byte unsigned char
#endif // byte

#ifndef NULL
#define NULL 0
#endif // NULL

#ifndef TRUE
#define TRUE 1
#endif // TRUE

#ifndef FALSE
#define FALSE 0
#endif // FALSE

// use POSIX message queue flag macro definition
#ifndef USE_POSIX_MQUEUE_FLAG
#define USE_POSIX_MQUEUE_FLAG
#endif // USE_POSIX_MQUEUE_FLAG

// constant value definition
#define LINUX_ERRNO_ERR_STR_BUF_LEN		(32)

// constant error code definition
#define RET_CODE_SUCCESS        		(0)
#define RET_CODE_FAIL           		(-1)
#define RET_CODE_INVALID_PARA			(-2)
#define RET_CODE_OUT_OF_MEM				(-3)
#define RET_CODE_QUEUE_FULL     		(1)
#define RET_CODE_QUEUE_EMPTY    		(2)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // COM_DEF_H
