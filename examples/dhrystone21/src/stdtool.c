/*****************************************************************************
 * @file
 * @author   Sergey Khabarov
 * @brief    Firmware example. 
 ****************************************************************************/

#include <inttypes.h>
#include <string.h>
#include <stdio.h>

extern char _end;

/**
 * @name sbrk
 * @brief Increase program data space.
 * @details Malloc and related functions depend on this.
 */
char *_sbrk(int incr) {
    return &_end;
}

//void _exit(int c) {
//    for (;;) { }
//}

int _kill(pid_t pid, int signum) {
    return 0;
}

int _getpid() {
    return 0;
}