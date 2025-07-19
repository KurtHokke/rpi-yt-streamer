#ifndef UTILS_H
#define UTILS_H

#include "help.h"

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "/usr"
#endif
#ifndef VENV_SITE_PACKAGES
#define VENV_SITE_PACKAGES "/usr/lib/python3.13/site-packages"
#endif
#define YT_DLP_PATH INSTALL_PREFIX "/bin/yt-dlp"
#define DL_PATH INSTALL_PREFIX "/dl_path"

#define PROG_WORKDIR "/.yt-dlp-server"
//#define LOG_FILE "/"

#define URL_MAX_LEN 256
#define OUTTMPL_MAX_LEN 128


static inline int check_start(int argc, char *argv[])
{
    if ((argc > 1) && (strcmp("--help", argv[1]) == 0)) {
        printf(PROGRAM_HELP_MSG);
        return 1;
    }
    if ((access(YT_DLP_PATH"/yt_dlp/__init__.py", F_OK) != 0) || (access(DL_PATH, F_OK | W_OK) != 0)) {
        printf(PROGRAM_HELP_MSG);
        return -1;
    }
    printf("check succeeded!\n");
    return 0;
}


#endif