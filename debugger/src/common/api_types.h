/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_COMMON_API_TYPES_H__
#define __DEBUGGER_COMMON_API_TYPES_H__

#include <inttypes.h>
#if defined(_WIN32) || defined(__CYGWIN__)
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <ws2tcpip.h>
#include <process.h>
#include <conio.h>  // console input/output
#else /* Linux */
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
#endif

namespace debugger {

#if defined(_WIN32) || defined(__CYGWIN__)
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
    typedef void *sharemem_def;   // HANDLE = void*

    #define RV_PRI64 "I64"
#else /* Linux */
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

static const uint64_t EXT_SIGN_5  = 0xFFFFFFFFFFFFFFF0LL;
static const uint64_t EXT_SIGN_6  = 0xFFFFFFFFFFFFFFE0LL;
static const uint64_t EXT_SIGN_8  = 0xFFFFFFFFFFFFFF80LL;
static const uint64_t EXT_SIGN_9  = 0xFFFFFFFFFFFFFF00LL;
static const uint64_t EXT_SIGN_11 = 0xFFFFFFFFFFFFF800LL;
static const uint64_t EXT_SIGN_12 = 0xFFFFFFFFFFFFF000LL;
static const uint64_t EXT_SIGN_16 = 0xFFFFFFFFFFFF0000LL;
static const uint64_t EXT_SIGN_32 = 0xFFFFFFFF00000000LL;

struct ECpuRegMapping {
    const char name[16];
    int size;
    uint32_t offset;
};

union Reg8Type {
    uint8_t byte;
    int8_t sbyte;
    struct bits_type {
        uint8_t b0 : 1;
        uint8_t b1 : 1;
        uint8_t b2 : 1;
        uint8_t b3 : 1;
        uint8_t b4 : 1;
        uint8_t b5 : 1;
        uint8_t b6 : 1;
        uint8_t b7 : 1;
    } bits;
};

union Reg16Type {
    uint8_t buf[2];
    uint16_t word;
    Reg8Type r8[2];
    struct bits_type {
        uint16_t b0 : 1;
        uint16_t b1 : 1;
        uint16_t b2 : 1;
        uint16_t b3 : 1;
        uint16_t b4 : 1;
        uint16_t b5 : 1;
        uint16_t b6 : 1;
        uint16_t b7 : 1;
        uint16_t b8 : 1;
        uint16_t b9 : 1;
        uint16_t b10 : 1;
        uint16_t b11 : 1;
        uint16_t b12 : 1;
        uint16_t b13 : 1;
        uint16_t b14 : 1;
        uint16_t b15 : 1;
    } bits;
};

union Reg32Type {
    uint8_t buf[4];
    uint16_t buf16[2];
    uint32_t val;
    int32_t  ival;
    float    f32;
    Reg8Type r8[4];
    Reg16Type r16[2];
    struct f32_bits_type {
        uint32_t mant : 23;
        uint32_t exp : 8;
        uint32_t sign : 1;
    } f32bits;
    struct bits_type {
        uint32_t b0 : 1;
        uint32_t b1 : 1;
        uint32_t b2 : 1;
        uint32_t b3 : 1;
        uint32_t b4 : 1;
        uint32_t b5 : 1;
        uint32_t b6 : 1;
        uint32_t b7 : 1;
        uint32_t b8 : 1;
        uint32_t b9 : 1;
        uint32_t b10 : 1;
        uint32_t b11 : 1;
        uint32_t b12 : 1;
        uint32_t b13 : 1;
        uint32_t b14 : 1;
        uint32_t b15 : 1;
        uint32_t b16 : 1;
        uint32_t b17 : 1;
        uint32_t b18 : 1;
        uint32_t b19 : 1;
        uint32_t b20 : 1;
        uint32_t b21 : 1;
        uint32_t b22 : 1;
        uint32_t b23 : 1;
        uint32_t b24 : 1;
        uint32_t b25 : 1;
        uint32_t b26 : 1;
        uint32_t b27 : 1;
        uint32_t b28 : 1;
        uint32_t b29 : 1;
        uint32_t b30 : 1;
        uint32_t b31 : 1;
    } bits;
};

union Reg64Type {
    uint8_t buf[8];
    uint16_t buf16[4];
    uint32_t buf32[2];
    uint64_t val;
    int64_t  ival;
    float    f32[2];
    double   f64;
    Reg8Type r8[8];
    Reg16Type r16[4];
    Reg32Type r32[2];
    struct f64_bits_type {
        uint64_t mant : 52;
        uint64_t exp : 11;
        uint64_t sign : 1;
    } f64bits;
    struct bits_type {
        uint64_t b0 : 1;
        uint64_t b1 : 1;
        uint64_t b2 : 1;
        uint64_t b3 : 1;
        uint64_t b4 : 1;
        uint64_t b5 : 1;
        uint64_t b6 : 1;
        uint64_t b7 : 1;
        uint64_t b8 : 1;
        uint64_t b9 : 1;
        uint64_t b10 : 1;
        uint64_t b11 : 1;
        uint64_t b12 : 1;
        uint64_t b13 : 1;
        uint64_t b14 : 1;
        uint64_t b15 : 1;
        uint64_t b16 : 1;
        uint64_t b17 : 1;
        uint64_t b18 : 1;
        uint64_t b19 : 1;
        uint64_t b20 : 1;
        uint64_t b21 : 1;
        uint64_t b22 : 1;
        uint64_t b23 : 1;
        uint64_t b24 : 1;
        uint64_t b25 : 1;
        uint64_t b26 : 1;
        uint64_t b27 : 1;
        uint64_t b28 : 1;
        uint64_t b29 : 1;
        uint64_t b30 : 1;
        uint64_t b31 : 1;
    } bits;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_API_TYPES_H__
