/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdarg.h>

const char LOG_FILE[] = "dpilib.log";
FILE *fp = 0;

extern "C" void LIB_thread_create(void *data) {
    LibThreadType *p = (LibThreadType *)data;
#if defined(_WIN32) || defined(__CYGWIN__)
    p->Handle = (thread_def)_beginthreadex(0, 0, p->func, p->args, 0, 0);
#else
    pthread_create(&p->Handle, 0, p->func, p->args);
#endif
}

extern "C" uint64_t LIB_thread_id() {
	  uint64_t r;
#if defined(_WIN32) || defined(__CYGWIN__)
	  r = GetCurrentThreadId();
#else
	  r = pthread_self();
#endif
	  return r;
}

extern "C" void LIB_thread_join(thread_def th, int ms) {
#if defined(_WIN32) || defined(__CYGWIN__)
    WaitForSingleObject(th, ms);
#else
    pthread_join(th, 0);
#endif
}

extern "C" void LIB_sleep_ms(int ms) {
#if defined(_WIN32) || defined(__CYGWIN__)
    Sleep(ms);
#else
    usleep(1000*ms);
#endif
}

extern "C" void LIB_event_create(event_def *ev, const char *name) {
#if defined(_WIN32) || defined(__CYGWIN__)
    ev->state = false;
    ev->cond = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT(name)    // object name
        );
#else
    pthread_mutex_init(&ev->mut, NULL);
    pthread_cond_init(&ev->cond, NULL);
    ev->state = false;
#endif
}

extern "C" void LIB_event_close(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    CloseHandle(ev->cond);
#else
    pthread_mutex_destroy(&ev->mut);
    pthread_cond_destroy(&ev->cond);
#endif
}

extern "C" void LIB_event_set(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    ev->state = true;
    SetEvent(ev->cond);
#else
    pthread_mutex_lock(&ev->mut);
    ev->state = true;
    pthread_mutex_unlock(&ev->mut);
    pthread_cond_signal(&ev->cond);
#endif
}

extern "C" int LIB_event_is_set(event_def *ev) {
    return ev->state ? 1: 0;
}

extern "C" void LIB_event_clear(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    ev->state = false;
    ResetEvent(ev->cond);
#else
    ev->state = false;
#endif
}

extern "C" void LIB_event_wait(event_def *ev) {
#if defined(_WIN32) || defined(__CYGWIN__)
    WaitForSingleObject(ev->cond, INFINITE);
#else
    int result = 0;
    pthread_mutex_lock(&ev->mut);
    while (result == 0 && !ev->state) {
        result = pthread_cond_wait(&ev->cond, &ev->mut);
    } 
    pthread_mutex_unlock(&ev->mut);
#endif
}

extern "C" int LIB_event_wait_ms(event_def *ev, int ms) {
#if defined(_WIN32) || defined(__CYGWIN__)
    DWORD wait_ms = ms;
    if (ms == 0) {
        wait_ms = INFINITE;
    }
    if (WAIT_TIMEOUT == WaitForSingleObject(ev->cond, wait_ms)) {
        return 1;
    }
    return 0;
#else
    struct timeval tc;
    struct timespec ts;
    int next_us;
    int result = 0;
    gettimeofday(&tc, NULL);
    next_us = tc.tv_usec + 1000 * ms;
    ts.tv_sec = tc.tv_sec + (next_us / 1000000);
    next_us -= 1000000 * (next_us / 1000000);
    ts.tv_nsec = 1000 * next_us;

    pthread_mutex_lock(&ev->mut);
    while (result == 0 && !ev->state) {
        result = pthread_cond_timedwait(&ev->cond, &ev->mut, &ts);
    }
    pthread_mutex_unlock(&ev->mut);
    if (ETIMEDOUT == result) {
        return 1;
    }
    return 0;
#endif
}


extern "C" int LIB_printf(const char *fmt, ...) {
    va_list arg;
    char tstr[4096];
    int ret = 0;
    va_start(arg, fmt);
#if defined(_WIN32) || defined(__CYGWIN__)
    ret += vsnprintf(tstr, sizeof(tstr), fmt, arg);
#else
    ret += vsprintf(tstr, fmt, arg);
#endif
    va_end(arg);

    if (fp == 0) {
        fp = fopen(LOG_FILE, "wb");
        if (!fp) {
            printf("Can't open file %s\n", LOG_FILE);
            return 0;
        }
    }
    fwrite(tstr, ret, 1, fp);
    fflush(fp);
    return ret;
}

extern "C" int LIB_sprintf(char *s, size_t len, const char *fmt, ...) {
    int ret;
    va_list arg;
    va_start(arg, fmt);
#if defined(_WIN32) || defined(__CYGWIN__)
    ret = vsprintf_s(s, len, fmt, arg);
#else
    ret = vsprintf(s, fmt, arg);
#endif
    va_end(arg);
    return ret;
}


extern "C" void *LIB_get_proc_addr(const char *f) {
    void *ret = 0;
#if defined(_WIN32) || defined(__CYGWIN__)
    DWORD processID = GetCurrentProcessId();
    DWORD cbNeeded;
    HMODULE hMods[1024];
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                            PROCESS_VM_READ,
                            FALSE, processID );
    if (!hProcess) {
        LIB_printf("Can't open process to get %s\n", f);
        return 0;
    }

    // Get List of all modules in a process
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (int i = 0; i < cbNeeded / sizeof(HMODULE); i++) {
            TCHAR wModName[MAX_PATH];
            ret = (void *)GetProcAddress(hMods[i], f);
            if (ret) {
                GetModuleFileNameEx(hProcess, hMods[i], wModName,
                        sizeof(wModName) / sizeof(TCHAR));
                LIB_printf("Symbol %s found in module %s\n", f, wModName);
                break;
            }
        }
    }

    if (!ret) {
        LIB_printf("Function %s not found\n", f);
    }
    CloseHandle(hProcess);
#else
    void *hproc = dlopen(0, RTLD_LAZY);
    ret = dlsym(hproc, f);
    if (ret == 0) {
        LIB_printf("Symbol %s not found\n", f);
    }
    dlclose(hproc);
#endif
    return ret;
}


