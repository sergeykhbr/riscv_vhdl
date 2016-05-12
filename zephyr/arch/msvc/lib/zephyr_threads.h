#pragma once

#include <vector>
#include <inttypes.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <process.h>

typedef void *_thread_arg_t;
typedef void (*_thread_entry_t)(_thread_arg_t arg1,
							  _thread_arg_t arg2,
							  _thread_arg_t arg3);

typedef void (*_thread_entry_asm)(_thread_entry_t pEntry,
					_thread_arg_t parameter1,
					_thread_arg_t parameter2,
					_thread_arg_t parameter3);

struct ThreadDataType {
    int idx;
    uint64_t stack_offset;
    _thread_entry_asm func;
    _thread_entry_t entry;
    _thread_arg_t args[3];
    int priority;
    unsigned options;
    HANDLE hThread;
    HANDLE hEvent;
    bool preemptive;
    uint64_t return_value;
};


extern "C" int LIBH_create_thread(char *pStackMem,
                                unsigned stackSize, 
                                int priority, 
                                unsigned options);
extern "C" unsigned int LIBH_swap(uint64_t stack);
