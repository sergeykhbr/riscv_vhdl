/**
 * https://sites.google.com/site/stm32discovery/open-source-development-with-the-stm32-discovery/getting-newlib-to-work-with-stm32-and-code-sourcery-lite-eabi
 */
#include <errno.h>
#include <sys/stat.h>
#ifdef WIN32
    typedef size_t _ssize_t;
    static const int STDOUT_FILENO = 0;
    static const int STDERR_FILENO = 1;
    static const int STDIN_FILENO = 2;
#else
    #include <sys/times.h>
    #include <sys/unistd.h>
#endif
#include <cstdio>
#include <axi_maps.h>

#ifdef WIN32
char _end[1<<19];
#else
extern char _end;  // defined in linker script
#endif
static uint64_t pool_size=0;

#ifndef STDOUT_USART
#define STDOUT_USART 2
#endif

#ifndef STDERR_USART
#define STDERR_USART 2
#endif

#ifndef STDIN_USART
#define STDIN_USART 2
#endif

#undef errno
int errno;

extern "C" void __cxa_pure_virtual() {
    while (1);
}

/*
 environ
 A pointer to a list of environment variables and their values. 
 For a minimal environment, this empty list is adequate:
 */
char *__env[1] = { 0 };
char **environ = __env;

/**
 * @brief   Read a character to a file.
 * @details `libc' subroutines will use this system routine for input from
 *          all files, including stdin
 * @return  -1 on error or blocks until the number of characters have been
            read.
 */
extern "C" _ssize_t _read(int file, void *ptr, size_t len) {
    return 0;
}

/**
 * @brief   Write a character to a file. 
 * @details `libc' subroutines will use this system routine for output 
 *          to all files, including stdout
 * @return -1 on error or number of bytes sent
 */
extern "C" _ssize_t _write(int file, const void *ptr, size_t len) {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    uint8_t *buf = (uint8_t *)ptr;
    size_t total = len;
    while (total) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        uart->data = *buf;
        total--;
        buf++;
    }
    return len;
}


/*
 execve
 Transfer control to a new process. Minimal implementation (for a system without processes):
 */
extern "C" int _execve(char *name, char **argv, char **env) {
    errno = ENOMEM;
    return -1;
}
/*
 fork
 Create a new process. Minimal implementation (for a system without processes):
 */

extern "C" int _fork() {
    errno = EAGAIN;
    return -1;
}

#if!defined(WIN32)
/*
 fstat
 Status of an open file. For consistency with other minimal implementations in these examples,
 all files are regarded as character special devices.
 The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
 */
extern "C" int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

extern "C" void _exit(int status) {
    write(1, (char *)"exit", 4);
    while (1) {
        ;
    }
}
#endif

extern "C" int _close(int file) {
    return -1;
}

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */

extern "C" int getpid() {
    return 1;
}

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
extern "C" int _isatty(int file) {
    switch (file){
    case STDOUT_FILENO:
    case STDERR_FILENO:
    case STDIN_FILENO:
        return 1;
    default:
        //errno = ENOTTY;
        errno = EBADF;
        return 0;
    }
}


/*
 kill
 Send a signal. Minimal implementation:
 */
extern "C" int kill(int pid, int sig) {
    errno = EINVAL;
    return (-1);
}

/*
 link
 Establish a new name for an existing file. Minimal implementation:
 */

extern "C" int _link(char *old, char *new_) {
    errno = EMLINK;
    return -1;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
extern "C" off_t _lseek(int file, off_t ptr, int dir) {
    return 0;
}


/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */
extern "C" char *_sbrk(int incr) {
    uint64_t misbyte = 0x8 - (((uint64_t)&_end + pool_size) & 0x7);
    pool_size += (misbyte & 0x7);

    char *ret = (char *)((uint64_t)&_end + pool_size);
    pool_size += incr;
#if 0
    WRITE64(&((pnp_map *)ADDR_BUS0_XSLV_PNP)->malloc_addr, (uint64_t)ret);
    WRITE64(&((pnp_map *)ADDR_BUS0_XSLV_PNP)->malloc_size, (uint64_t)size);
#endif
    return ret;  
}
