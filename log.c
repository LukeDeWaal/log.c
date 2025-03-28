/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "log.h"


#define MAX_CALLBACKS 32

typedef struct {
    log_LogFn fn;
    void *udata;
    int level;
} Callback;

static struct {
    void *udata;
    log_LockFn lock;
    int level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} Logger;


static const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif


static void stdout_callback(log_Event *ev) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "[%H:%M:%S]", ev->time)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(
      ev->udata, "%s %s%-5s\x1b[0m \x1b[97m%s:%04d (%s):\x1b[0m ",
      buf, level_colors[ev->level], level_strings[ev->level],
      ev->file, ev->line, ev->func);
#else
    fprintf(
            ev->udata, "%s %-5s %s:%04d: ",
            buf, level_strings[ev->level], ev->file, ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}


static void file_callback(log_Event *ev) {
    char buf[64];
    buf[strftime(buf, sizeof(buf), "[%H:%M:%S]", ev->time)] = '\0';
    fprintf(
            ev->udata, "%s %-5s %s:%d (%s): ",
            buf, level_strings[ev->level], ev->file, ev->line, ev->func);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}


static void lock(void) {
    if (Logger.lock) { Logger.lock(true, Logger.udata); }
}


static void unlock(void) {
    if (Logger.lock) { Logger.lock(false, Logger.udata); }
}


const char *log_level_string(int level) {
    return level_strings[level];
}


void log_set_lock(log_LockFn fn, void *udata) {
    Logger.lock = fn;
    Logger.udata = udata;
}


void log_set_level(int level) {
    Logger.level = level;
}

int log_get_level(void) {
    return Logger.level;
}

void log_set_quiet(bool enable) {
    Logger.quiet = enable;
}


int log_add_callback(log_LogFn fn, void *udata, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!Logger.callbacks[i].fn) {
            Logger.callbacks[i] = (Callback) {fn, udata, level};
            return 0;
        }
    }
    return -1;
}


int log_add_fp(FILE *fp, int level) {
    return log_add_callback(file_callback, fp, level);
}


static void init_event(log_Event *ev, void *udata) {
    if (!ev->time) {
        time_t t = time(NULL);
        ev->time = localtime(&t);
    }
    ev->udata = udata;
}


void log_log(int level, const char *file, int line, const char* func, const char *fmt, ...) {

    log_Event ev = {
            .fmt   = fmt,
            .file  = file,
            .line  = line,
            .func  = func,
            .level = level,
    };

    lock();

    if (!Logger.quiet && level >= Logger.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && Logger.callbacks[i].fn; i++) {
        Callback *cb = &Logger.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}
