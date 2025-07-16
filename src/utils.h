#ifndef UTILS_H
#define UTILS_H

#include <jansson.h>

json_t *dl_ytdlp(char *url);

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "/usr"
#endif
#ifndef VENV_SITE_PACKAGES
#define VENV_SITE_PACKAGES "/usr/lib/python3.13/site-packages"
#endif
#define YT_DLP_PATH INSTALL_PREFIX "/bin/yt-dlp"
#define DL_PATH INSTALL_PREFIX "/dl_path"

#endif