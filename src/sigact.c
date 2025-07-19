
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
            puts("shutting down server_fd");
            shutdown(ctx->server_fd, SHUT_RDWR);
        }
        if (ctx->Py.main_tstate) {
            PyEval_RestoreThread(ctx->Py.main_tstate);
        }
        if (Py_IsInitialized()) Py_Finalize();
        if (ctx->log) {
            puts("closing logfile");
            fclose(ctx->log);
        }
        free(ctx);
    }
    exit(e);
}

void handle_sigint(int sig)
{
    psignal(sig, "\nReceived signal");
    switch(sig) {
        case SIGINT:
        case SIGTERM:
            free_all_exit(sig, "");
            break;
        default:
            break;
    }
    //free_all_exit(sig, "");
}

void setup_signal_handler(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    //sa.sa_flags = 0;
    sa.sa_flags = SA_RESTART;
    
    // Block other signals during handler execution
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGHUP);
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set up SIGINT handler");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Failed to set up SIGTERM handler");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction(SIGHUP)");
        exit(EXIT_FAILURE);
    }
    signal(SIGPIPE, SIG_IGN);
}