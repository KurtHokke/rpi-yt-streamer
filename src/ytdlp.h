#ifndef YTDLP_H
#define YTDLP_H

#include <Python.h>
#include <jansson.h>

// Global Python objects
extern PyObject *pModule;
extern PyObject *pJsonModule;
extern PyObject *pClass;

// Function prototypes
int ytdlp_init(void);
void ytdlp_finalize(void);
//int extract_info_and_parse(const char *url);

#endif