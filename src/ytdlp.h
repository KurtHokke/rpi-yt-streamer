#ifndef YTDLP_H
#define YTDLP_H

#include "utils.h"
#include <jansson.h>

/*Struct to hold opts for when calling dl_ytdlp()*/
struct ytdlp_opts_t {
    char url[URL_MAX_LEN];
    char outtmpl[OUTTMPL_MAX_LEN];
};

json_t *dl_ytdlp(struct ytdlp_opts_t *opts);
int ytdlp_init(void);
void ytdlp_finalize(void);
//int extract_info_and_parse(const char *url);

#endif