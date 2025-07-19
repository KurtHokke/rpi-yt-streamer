
#ifndef SIGACT_H
#define SIGACT_H

#include <Python.h>
#include <signal.h>

/*Global Python objects*/
struct Py {
    /*GIL*/
    PyThreadState *main_tstate;
    /*yt_dlp module*/
    PyObject *pModule;
    /*json module*/
    PyObject *pJsonModule;
    /*YoutubeDL class*/
    PyObject *pClass;
};
/**
 * @brief Global struct containing program context
 * 
 */
struct program_ctx_t {
    /*Keep running*/
    volatile sig_atomic_t keep_running;

    /*Server socket*/
    int server_fd;
    
    /*Log file*/
    FILE *log;

    struct Py Py;
};
extern struct program_ctx_t *ctx;

void thread_cleanup_handler(void *arg);
void free_all_exit(int e, const char *msg);
void setup_signal_handler(void);

#endif