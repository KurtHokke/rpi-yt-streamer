
#include "sigact.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>

void thread_cleanup_handler(void *arg)
{
    if (arg) {
        free(arg);
    }
    free_all_exit(16, "Thread error");
}

void free_all_exit(int e, const char *msg)
{
    if (ctx) {
        ctx->keep_running = 0;
        printf("%s\nshutting down gracefully...\n", msg);
        if (ctx->server_fd != -1) {
            shutdown(ctx->server_fd, SHUT_RDWR);
        }
        if (ctx->Py.main_tstate) {
            PyEval_RestoreThread(ctx->Py.main_tstate);
        }
        Py_Finalize();
        free(ctx);
    }
    exit(e);
}

void handle_sigint(int sig)
{
    psignal(sig, "\nReceived signal");
    free_all_exit(sig, "");
}

void setup_signal_handler(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    ;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set up SIGINT handler");
        exit(1);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Failed to set up SIGTERM handler");
        exit(1);
    }
    signal(SIGPIPE, SIG_IGN);
}