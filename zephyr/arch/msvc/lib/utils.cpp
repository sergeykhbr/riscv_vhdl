/*
 * Copyright (c) 2016, GNSS Sensor Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>
#include <iostream>
#include "zephyr_threads.h"
#include "irqctrl.h"
#include "uart.h"
#include "pnp.h"

extern IrqController irqctrl;
extern Uart uart0;
extern PNP pnp;
extern volatile bool wasPreemtiveSwitch;
extern int current_idx;
extern std::vector<ThreadDataType> vecThreads;

extern "C" int LIBH_uart_output(int v) {
    std::cout << (char)v;
    return 0;
}

DWORD WINAPI thread_wrapper(LPVOID param) {
    unsigned idx = ((unsigned )param);
    //printf_s("<thread_wrapper> Event[%d] waiting\n", idx);
    WaitForSingleObject(vecThreads[idx].hEvent, INFINITE);
    ResetEvent(vecThreads[idx].hEvent);
    //printf_s("<thread_wrapper> Event[%d] received\n", idx);
    vecThreads[idx].func(vecThreads[idx].entry,
                         vecThreads[idx].args[0],
                         vecThreads[idx].args[1],
                         vecThreads[idx].args[2]
                         );
    printf_s("<%s> End of thread[%d]!!\n", __FUNCTION__, idx);
    return 0;
}

extern "C" int LIBH_create_thread(char *pStackMem,
                              unsigned stackSize, 
                              int priority, 
                              unsigned options) {
    //unsigned long * ctx = (unsigned long *)(pStackMem + stackSize);
    tcs_simple *ctx = (tcs_simple *)pStackMem;
    ThreadDataType data;
    data.idx = (int)vecThreads.size();
    data.stack_offset = (uint64_t)pStackMem;

    data.func = (_thread_entry_asm)ctx->regs[REG_RA];
    data.entry = (_thread_entry_t)ctx->regs[REG_A0];
    data.args[0] = (_thread_arg_t)ctx->regs[REG_A1];
    data.args[1] = (_thread_arg_t)ctx->regs[REG_A2];
    data.args[2] = (_thread_arg_t)ctx->regs[REG_A3];
    data.priority = priority;
    data.options = options;
    data.preemptive = false;
    unsigned this_idx = vecThreads.size();

    data.hEvent = CreateEvent(  NULL,               // default security attributes
                                TRUE,               // manual-reset event
                                FALSE,              // initial state is nonsignaled
                                NULL          // object name
                                ); 

    data.hThread = CreateThread(NULL,
                            0,
                            thread_wrapper,
                            (LPVOID)this_idx,
                            CREATE_SUSPENDED,
                            NULL
                            );

    vecThreads.push_back(data);
    ResumeThread(data.hThread);
    return 0;
}

int LIBH_get_thread_by_context(uint64_t ctx) {
    ThreadDataType * ret = NULL;
    for (unsigned i = 0; i < vecThreads.size(); i++) {
        if (vecThreads[i].stack_offset == ctx) {
            return (int)i;
        }
    }
    return -1;
}

extern "C" void LIBH_swap(uint64_t stack) {
    int next_idx = LIBH_get_thread_by_context(stack);

    if (next_idx == current_idx) {
        // Find preemptive halted thread:
        for (unsigned i = 0; i < vecThreads.size(); i++) {
            if (vecThreads[i].preemptive) {
                next_idx = (int)i;
                break;
            }
        }
    }

    if (next_idx != current_idx) {
        int tmp_idx = current_idx;
        current_idx = next_idx;
        if (vecThreads[next_idx].preemptive) {
            vecThreads[next_idx].preemptive = false;
            ResumeThread(vecThreads[next_idx].hThread);
        } else {
            SetEvent(vecThreads[next_idx].hEvent);
        }
        vecThreads[tmp_idx].preemptive = false;
        WaitForSingleObject(vecThreads[tmp_idx].hEvent, INFINITE);
        ResetEvent(vecThreads[tmp_idx].hEvent);
    } else {
     
    }
}

extern "C" void LIBH_swap_preemptive(uint64_t ctx_addr) {
    int next_idx = LIBH_get_thread_by_context(ctx_addr);
    vecThreads[current_idx].preemptive = true;
    if (next_idx != -1) {
        tcs_simple *ctx = (tcs_simple *)vecThreads[next_idx].stack_offset;
        current_idx = next_idx;
        if (vecThreads[next_idx].preemptive) {
            ResumeThread(vecThreads[next_idx].hThread);
        } else {
            SetEvent(vecThreads[next_idx].hEvent);
        }
    }
    wasPreemtiveSwitch = true;
}


extern "C" void LIBH_write(uint64_t addr, uint8_t *buf, int size) {
    if (addr >= 0x80000000 && addr < 0x80001000) {
        // GPIO
    } else if (uart0.isAddrValid(addr)) {
        uart0.write(addr, buf, size);
    } else if (irqctrl.isAddrValid(addr)) {
        irqctrl.write(addr, buf, size);
    } else if (pnp.isAddrValid(addr)) {
        pnp.write(addr, buf, size);
    } else if (addr >= 0x80005000 && addr < 0x80006000) {
        // GP timers
    } else {
        std::cout << "Unmapped access\n";
    }
}

extern "C" void LIBH_read(uint64_t addr, uint8_t *buf, int size) {
    if (addr >= 0x80000000 && addr < 0x80001000) {
        // GPIO
    } else if (uart0.isAddrValid(addr)) {
        uart0.read(addr, buf, size);
    } else if (irqctrl.isAddrValid(addr)) {
        irqctrl.read(addr, buf, size);
    } else if (pnp.isAddrValid(addr)) {
        pnp.read(addr, buf, size);
    } else if (addr >= 0x80005000 && addr < 0x80006000) {
        // GP timers
    } else {
        std::cout << "Unmapped access\n";
    }
}
