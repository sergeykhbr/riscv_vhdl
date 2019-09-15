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

#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <ws2tcpip.h>
#include <process.h>
#include <Psapi.h>

typedef SOCKET socket_def;
typedef int addr_size_t;

typedef CRITICAL_SECTION mutex_def;
typedef void *thread_def;   // HANDLE = void*
typedef struct event_def {
    void *cond;             // HANDLE = void*
    bool state;
} event_def;
typedef unsigned thread_return_t;
typedef thread_return_t (__stdcall* lib_thread_func)(void *args);

#define RV_PRI64 "I64"

#else

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#define __STDC_FORMAT_MACROS
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>  // work with console
#include <errno.h>
#include <dlfcn.h>

typedef int socket_def;
typedef unsigned int addr_size_t;
typedef pthread_t thread_def;
typedef pthread_mutex_t mutex_def;
typedef struct event_def {
    pthread_mutex_t mut;
    pthread_cond_t cond;
    bool state;
} event_def;
typedef void *thread_return_t;
typedef thread_return_t (*lib_thread_func)(void *args);
typedef int sharemem_def;

# if defined(__WORDSIZE) && (__WORDSIZE == 64)
#  define RV_PRI64 "l"
# else
#  define RV_PRI64 "ll"
# endif

#endif

typedef struct LibThreadType {
    lib_thread_func func;
    void *args;
    thread_def Handle;
} LibThreadType;

