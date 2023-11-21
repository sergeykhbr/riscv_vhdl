/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <inttypes.h>

extern char __heap_start__[];  // defined in linker script

/*
 * @brief Increase program data space.
 * @details Malloc and related functions depend on this
 */
char * __attribute__((weak)) _sbrk(int incr) {
    return __heap_start__;
}


/**
 * @brief   Write a character to a file. 
 * @details `libc' subroutines will use this system routine for output 
 *          to all files, including stdout
 * @return -1 on error or number of bytes sent
 */
int _write(int file __attribute__((unused)),
           const void *ptr __attribute__((unused)),
           int len)
{
    return len;
}

int _close(int file __attribute__((unused)))
{
    return -1;
}

#if!defined(_WIN32)
void __attribute__((weak)) _exit(int code __attribute__((unused)))
{
  // TODO: write on trace
  while (1) {}
}
#endif

/*
 fstat
 Status of an open file. For consistency with other minimal implementations in these examples,
 all files are regarded as character special devices.
 The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
 */
int _fstat(int file __attribute__((unused)),
           void *st __attribute__((unused)))
{
    return 0;
}

int __attribute__((weak))_getpid(void)
{
  return -1;
}

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
int _isatty(int file __attribute__((unused)))
{
    return 1;
}

int __attribute__((weak))
_kill(int pid __attribute__((unused)), int sig __attribute__((unused)))
{
  return -1;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
int _lseek(int file __attribute__((unused)),
             int ptr __attribute__((unused)),
             int dir __attribute__((unused)))
{
    return 0;
}

/**
 * @brief   Read a character to a file.
 * @details `libc' subroutines will use this system routine for input from
 *          all files, including stdin
 * @return  -1 on error or blocks until the number of characters have been
            read.
 */
int _read(int file __attribute__((unused)),
               void *ptr __attribute__((unused)),
               int len __attribute__((unused)))
{
    return 0;
}
