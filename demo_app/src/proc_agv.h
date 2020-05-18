#ifndef PROC_ARG_H
#define PROC_ARG_H

struct globalArgs_t
{
    int is_daemon;
    int is_svr;
    int sock_fd;
    int debug;
};

int proc_arg(int argc, char* argv[]);
#endif //PROC_ARG_H