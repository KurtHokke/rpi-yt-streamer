#define _GNU_SOURCE
#include <Python.h>
#include "ytdlp.h"
#include "utils.h"
#include "help.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "sigact.h"
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vlc/vlc.h>
#include <jansson.h>
#include <glob.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 4096

struct program_ctx_t *ctx = NULL;
struct program_ctx_t *ctx_init(void) {
    struct program_ctx_t *ctx_i = malloc(sizeof(*ctx_i));
    *ctx_i = (struct program_ctx_t) {
        .keep_running = 1,
        .server_fd = -1,
        .log = NULL,
        .Py = {0}
    };
    return ctx_i;
}



void *parse_metadata(void *arg)
{
    json_t *metadata = (json_t *)arg;
    pthread_cleanup_push(thread_cleanup_handler, metadata);
    json_t *tb_array = json_object_get(metadata, "thumbnails");
    if (!tb_array) {
        printf("!tb_array\n");
        json_decref(metadata);
        metadata = NULL;
        pthread_exit(NULL);
    }
    printf("Array size: %lu\n", json_array_size(tb_array));
    json_t *tb_value = NULL;
    json_t *tb_filepath = NULL;

    for (ssize_t i = (ssize_t)json_array_size(tb_array) - 1;
         i >= 0 && (tb_value = json_array_get(tb_array, (size_t)i));
         i--) {

        if ((tb_filepath = json_object_get(tb_value, "filepath"))) {
            printf("tb_filepath: %s\n", json_string_value(tb_filepath));
            break;
        }
    }
    json_dump_file(metadata, "janssondump.json", JSON_INDENT(4));
    json_decref(metadata);
    metadata = NULL;
    pthread_cleanup_pop(0);
    return (void *)0;
}
void *handle_client(void *arg)
{
    int *client_fd = (int *)arg;
    pthread_cleanup_push(thread_cleanup_handler, client_fd);
    
    struct ytdlp_opts_t *opts = malloc(sizeof(*opts));
    if (!opts) {
        perror("malloc failed");
        close(*client_fd);
        pthread_exit(NULL);
    }

    ssize_t bytes_read = read(*client_fd, opts->url, URL_MAX_LEN - 1);
    if (bytes_read <= 0) {
        printf("failed to read buf\n");
        close(*client_fd);
        free(opts);
        pthread_exit(NULL);
    }
    opts->url[bytes_read] = '\0';

    json_t *metadata = dl_ytdlp(opts);
    if (!metadata) {
        printf("\n\nFAAAAAIIILLLL\n\n");
        close(*client_fd);
        free(opts);
        pthread_exit(NULL);
    }
    pthread_t parse_metadata_thread;
    void *ret_pm_thread;
    pthread_create(&parse_metadata_thread, NULL, parse_metadata, (void *)metadata);
    if (pthread_join(parse_metadata_thread, &ret_pm_thread) != 0) {
        perror("pthread_join() failed");
        json_decref(metadata);
        close(*client_fd);
        free(opts);
        pthread_exit(NULL);
    }
    if ((int *)ret_pm_thread == 0) {
        printf("Successfully parsed metadata!!!\n");
    } else {
        printf("FAIL!!!!!!\n");
    }

    close(*client_fd);
    free(opts);
    free(client_fd);
    pthread_cleanup_pop(0);
    return (void *)0;
}

int main(int argc, char *argv[])
{
    int check_ret = check_start(argc, argv);
    if (check_ret != 0) return check_ret;

    setup_signal_handler();
    ctx = ctx_init();

    // Initialize Python once
    Py_Initialize();
    ctx->Py.main_tstate = PyEval_SaveThread();  // Release GIL
    
    int l_client_fd = -1;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 1. Create socket (IPv4, TCP)
    ctx->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->server_fd < 0) {
        free_all_exit(errno, "socket() failed");
    }

    // 2. Set socket options (reuse address)
    if (setsockopt(ctx->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        free_all_exit(1, "setsockopt() failed");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 3. Bind socket to port
    if (bind(ctx->server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        free_all_exit(1, "bind() failed");
    }

    // 4. Start listening
    if (listen(ctx->server_fd, 3) < 0) {
        free_all_exit(1, "listen() failed");
    }
    printf("Server listening on port %d...\n", PORT);

    // 5. Accept incoming connections
    while (ctx->keep_running) {
        if ((l_client_fd = accept(ctx->server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            if (!ctx->keep_running) break;
            perror("accept failed");
            continue; // Keep trying even if one client fails
        } else {
            int *client_fd = malloc(sizeof(int));
            *client_fd = l_client_fd;
            pthread_t thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            puts("1");
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            puts("2");
            if (pthread_create(&thread, &attr, handle_client, (void *)client_fd) != 0) {
                close(*client_fd);
                free(client_fd);
                free_all_exit(1, "Failed to create thread");
            }
            pthread_attr_destroy(&attr);
            puts("successfully detached thread!!");
        }
    }
    while (1) {
        usleep(1000000);
    }
    return 0;
}