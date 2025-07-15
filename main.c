#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vlc/vlc.h>
#include <jansson.h>
#include <glob.h>
#include <errno.h> // For errno

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "/usr/local"
#endif
#define YT_DLP_PATH INSTALL_PREFIX "/bin/yt-dlp"
#define DL_PATH INSTALL_PREFIX "/dl_path"

#define PORT 8080
#define BUFFER_SIZE 1024

char *match_ext(char **pathv, const char **extv)
{
    char *ret = NULL;
    for (int i = 0; pathv[i]; i++) {
        size_t path_len = strlen(pathv[i]);
        for (int j = 0; extv[j]; j++) {
            size_t ext_len = strlen(extv[j]);
            if (strcasecmp(pathv[i] + path_len - ext_len, extv[j]) == 0) {
                ret = strdup(pathv[i]);
                return ret;
            }
        }
    }
    return NULL;
}

char **find_glob(const char *pattern)
{
    char **ret = NULL;
    //size_t match_count = 0;
    glob_t results;
    if (glob(pattern, GLOB_TILDE, NULL, &results) == 0) {

        ret = calloc(results.gl_pathc + 1, sizeof(char *));
        if (!ret) {
            globfree(&results);
            return NULL;
        }
        for (size_t i = 0; i < results.gl_pathc; i++) {
            const char *path = results.gl_pathv[i];
            printf("match: %zu,%s\n", i, path);
            
            ret[i] = strdup(path);
            if (!ret[i]) {
                for (size_t j = 0; j < i; j++) {
                    free(ret[j]);
                }
                free(ret);
                globfree(&results);
                return NULL;
            }
        }
        globfree(&results);
    }
    return ret;
}

void *handle_vlc_playback(void *arg)
{
    int sock = *(int *)arg;
    free(arg); // Free the socket pointer
    
    setenv("DISPLAY", ":0", 1);

    // Initialize VLC
    libvlc_instance_t *inst = libvlc_new(0, NULL);
    if (!inst) {
        fprintf(stderr, "VLC init failed\n");
        close(sock);
        pthread_exit(NULL);
    }

    // Play the downloaded file (modify path as needed)
    char video_path[PATH_MAX];
    snprintf(video_path, sizeof(video_path), "%s/*", DL_PATH); // Match yt-dlp's output
    
    libvlc_media_player_t *mp = libvlc_media_player_new(inst);
    libvlc_media_t *media = libvlc_media_new_path(inst, video_path);
    
    libvlc_media_player_set_media(mp, media);
    libvlc_media_player_play(mp);

    // Playback control loop
    while (libvlc_media_player_is_playing(mp)) {
        // Add your control logic here (pause/stop via socket messages)
        sleep(1);
    }

    // Cleanup
    libvlc_media_release(media);
    libvlc_media_player_release(mp);
    libvlc_release(inst);
    close(sock);
    
    pthread_exit(NULL);
}

int dl_video(char *const args[], unsigned long unix_time)
{
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        printf("Child: about to run yt-dlp\n");

        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execv(YT_DLP_PATH, args);  // Replace child
        _exit(1);  // Only reached if execv fails
    }
    else if (pid > 0) {  // Parent (pid = child's PID)
        close(pipefd[1]);
        printf("Parent: launched child PID %d\n", pid);

        json_t *root = json_object();
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("%s\n", buffer);

            FILE *stream = fmemopen((void *)buffer, bytes_read, "r");
            if (!stream) {
                perror("fmemopen");
                return -1;
            }
            char line[256];
            int i = 0;
            while (fgets(line, sizeof(line), stream)) {
                line[strcspn(line, "\n")] = '\0';
                printf("read: %s\n", line);
                switch(i) {
                    case 0:
                        json_object_set_new(root, "title", json_string(line));
                        break;
                    case 1:
                        json_object_set_new(root, "extension", json_string(line));
                        break;
                    case 2:
                        json_object_set_new(root, "resolution", json_string(line));
                        break;
                    case 3:
                        json_object_set_new(root, "vcodec", json_string(line));
                        break;
                    case 4:
                        json_object_set_new(root, "acodec", json_string(line));
                        break;
                        
                    default:
                        break;
                }
                i++;
            }
            char jsonfile_name[PATH_MAX];
            snprintf(jsonfile_name, sizeof(jsonfile_name), DL_PATH"/%lu.json", unix_time);
            FILE *jsonfile = fopen(jsonfile_name, "w");
            if (!jsonfile) {
                perror("fopen jsonfile");
                json_decref(root);
                fclose(stream);
                return -1;
            }
            json_dumpf(root, jsonfile, JSON_INDENT(4));

            fclose(jsonfile);
            fclose(stream);
        }

        close(pipefd[0]);
        waitpid(pid, NULL, 0);
    } else {
        perror("fork failed");
        return -1;
    }
    return 0;
}

void *handle_client(void *arg)
{
    int sock = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};

    ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        printf("failed to read buf\n");
        pthread_exit(NULL);
    }
    buffer[bytes_read] = '\0';
    printf("Client: %s\n", buffer);

    unsigned long unix_time = (unsigned long)time(NULL);
    char outfile_path[PATH_MAX];
    snprintf(outfile_path, sizeof(outfile_path), DL_PATH"/%lu.%%(ext)s", unix_time);

    char *const args[] = {
        "yt-dlp",
        "-f", "bestvideo[height<=1080]+bestaudio/best[height<=1080]",
        "--write-thumbnail",
        //"-o", DL_PATH"/%(title)s [%(id)s].%(ext)s",
        "-o", outfile_path,
        "-O", "%(title)s\n%(ext)s\n%(resolution)s\n%(vcodec)s\n%(acodec)s\n%(channel)s\n%(channel_url)s",
        "--no-simulate",
        buffer,
        NULL
    };

    if (dl_video(args, unix_time) != 0) {
        pthread_exit(NULL);
    }

    char globfile[PATH_MAX];
    snprintf(globfile, sizeof(globfile), DL_PATH"/%lu*", unix_time);
    char **matches = find_glob(globfile);

    const char *extv[] = {
        "webp",
        "png",
        NULL
    };
    char *thumbn = match_ext(matches, extv);
    if (!thumbn) {
        printf("match ext failed\n");
    } else {
        printf("matched: %s\n", thumbn);
        free(thumbn);
    }

    int i;
    for (i = 0; matches[i]; i++) {
        printf("%s\n", matches[i]);
        free(matches[i]);
    }
    free(matches[i + 1]);
    free(matches);
    
    send(sock, buffer, bytes_read, 0);
/*
    pthread_t vlc_thread;
    int *vlc_sock = malloc(sizeof(int));
    *vlc_sock = sock;

    pthread_create(&vlc_thread, NULL, handle_vlc_playback, vlc_sock);
    pthread_detach(vlc_thread);
*/
    free(arg); // Free original socket pointer
    //pthread_exit(NULL); // Terminate this thread

    close(sock);
    pthread_exit(NULL);
}

int check_start(void)
{
    struct stat st;
    if (stat(YT_DLP_PATH, &st) == 0) {
        if (st.st_mode & S_IXUSR) {
            printf("✅ File exists and is executable!\n");
        } else {
            printf("❌ File exists but NOT executable.\n");
            if (chmod(YT_DLP_PATH, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0) {
                printf("✅ File is now executable!\n");
            } else {
                perror("❌ chmod() failed");
                return 1;
            }
        }
    } else {
        perror("❌ stat() failed");
        return 1;
    }
    if (stat(DL_PATH, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            if (st.st_mode & S_IRWXU) {
                printf("✅ Dir exists!\n");
            } else {
                printf("❌ Dir exists but NOT accessable.\n");
                return 1;
            }
        } else {
            printf("❌ File exists but NOT a dir.\n");
            return 1;
        }
    } else {
        printf("❌ Dir does not exist.\n");
        if (mkdir(DL_PATH, 0755) == 0) {
            printf("✅ Dir created successfully!\n");
        }
    }
    return 0;
}

int main()
{
    if (check_start() != 0) {
        printf("checkstart failed\n");
        return 1;
    }
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);


    // 1. Create socket (IPv4, TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket() failed");
        fprintf(stderr, "Error code: %d (%s)\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 2. Set socket options (reuse address)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    address.sin_port = htons(PORT);

    // 3. Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind() failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Start listening
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 5. Accept incoming connections
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue; // Keep trying even if one client fails
        } else {
            int *client_sock = malloc(sizeof(int));
            *client_sock = new_socket;
            pthread_t thread;
            pthread_create(&thread, NULL, handle_client, client_sock);
            pthread_detach(thread);
        }
    }

    return 0;
}