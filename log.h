/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <libgen.h>

#define LOG_VERSION "0.1.0"

typedef struct {
    va_list ap;
    const char *fmt;
    const char *file;
    const char *func;
    struct tm *time;
    void *udata;
    int line;
    int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);

typedef void (*log_LockFn)(bool lock, void *udata);

enum {
    LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL
};

#define LOG_MIN (LOG_TRACE)
#define LOG_MAX (LOG_FATAL)
#define BASEFILE basename(__FILE__)

#define log_trace(...) log_log(LOG_TRACE, BASEFILE, __LINE__, __func__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, BASEFILE, __LINE__, __func__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  BASEFILE, __LINE__, __func__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  BASEFILE, __LINE__, __func__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, BASEFILE, __LINE__, __func__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, BASEFILE, __LINE__, __func__, __VA_ARGS__)

const char *log_level_string(int level);

void log_set_lock(log_LockFn fn, void *udata);

void log_set_level(int level);

int log_get_level(void);

void log_set_quiet(bool enable);

int log_add_callback(log_LogFn fn, void *udata, int level);

int log_add_fp(FILE *fp, int level);

void log_log(int level, const char *file, int line, const char* func, const char *fmt, ...);

#endif
