/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Common library types definition.
 */

#ifndef __DEBUGGER_API_TYPES_H__
#define __DEBUGGER_API_TYPES_H__

#include <inttypes.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
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
#include <inttypes.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>  // work with console
#endif

namespace debugger {

#if defined(_WIN32) || defined(__CYGWIN__)
    typedef SOCKET socket_def;
    typedef int addr_size_t;

    typedef CRITICAL_SECTION mutex_def;
    typedef void *thread_def; // HANDLE = void*
    typedef struct event_def {
        void *cond; // HANDLE = void*
        bool state;
    } event_def;
    typedef unsigned thread_return_t;
    typedef thread_return_t (__stdcall* lib_thread_func)(void *args);

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

    # if defined(__WORDSIZE) && (__WORDSIZE == 64)
    #  define RV_PRI64 "l"
    # else
    #  define RV_PRI64 "ll"
    # endif
#endif

typedef struct LibThreadType
{
    lib_thread_func func;
    void *args;
    thread_def Handle;
} LibThreadType;

}  // namespace debugger

#endif  // __DEBUGGER_API_TYPES_H__
