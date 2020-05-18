#include <stdio.h>
#include <getopt.h>

#include "proc_agv.h"

extern struct globalArgs_t globalArgs;

static const char *optString = "Dscdh";
static struct option long_options[] =
{
    {"daemon",  no_argument, NULL, 'D'},
    {"server", no_argument, NULL, 's'},
    {"client", no_argument, NULL, 'c'},
    {"debug",  no_argument, NULL, 'd'},
    {"help", no_argument, NULL, 'h'},
    {NULL, no_argument, NULL, 0}
};

static void print_useage()
{
    printf("usage:\n");
    printf("file_sock [-D | --daemon] [-h | --help]\n");
    printf("\n  -D | --daemon \n"
           "\n  -s | --server \n"
           "\n  -c fd| --client fd \n"
           "\n  -h | --help \n"
           "    Show this message.\n"
           "\n");
}

int proc_arg(int argc, char* argv[])
{
    int opt;
    int option_index = 0;
    while ( (opt = getopt_long(argc, argv, optString, long_options, &option_index)) != EOF)
    {
        switch(opt)
        {
            case 'D':
            {
                globalArgs.is_daemon = 1;
                break;
            }
            case 'c':
            {
                globalArgs.is_svr = 0;
                break;
            }
            case 's':
            {
                globalArgs.is_svr = 1;
                break;
            }
            case 'd':
                globalArgs.debug = 1;
                break;
            case 'h':
            default:
                print_useage();
                return 0;
        }
    }
    if(globalArgs.is_svr == -1)
    {
        print_useage();
        return 0;
    }
    return 1;
}
