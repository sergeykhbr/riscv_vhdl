#include <fstream>
#include <iostream>
#include <conio.h>
#include "zephyr_threads.h"
#include "irqctrl.h"
#include "uart.h"

static const int IDLE_STACK_SIZE = sizeof(tcs_simple);
static char idle_stack[IDLE_STACK_SIZE];
IrqController irqctrl;
Uart uart0;
volatile bool wasPreemtiveSwitch;
int current_idx;
std::vector<ThreadDataType> vecThreads;

extern "C" void _SwapPreemptive(char *ctx);

extern "C" void idle_task_entry(_thread_entry_t pEntry,
					            _thread_arg_t arg1,
					            _thread_arg_t arg2,
					            _thread_arg_t arg3) {
    typedef void (*epoint)(void);
    epoint epoint_ = (epoint)pEntry;
    epoint_();
    printf_s("\n%s: End of thread!!\n", __FUNCTION__);
}


extern "C" void LIBH_create_dispatcher(void *entry_point) {
    DWORD tmp;
    tcs_simple *ctx = (tcs_simple *)idle_stack;
    ctx->regs[REG_RA] = (uint64_t)idle_task_entry;
    ctx->regs[REG_A0] = (uint64_t)entry_point;
    LIBH_create_thread(idle_stack, IDLE_STACK_SIZE, 0, 0);
    current_idx = 0;
    //printf_s("Event[%d] idle_tick set\n", current_idx);
    SetEvent(vecThreads[current_idx].hEvent);

    while (1) {
        tmp = SuspendThread(vecThreads[current_idx].hThread);
        wasPreemtiveSwitch = false;

        irqctrl.raise_interrupt(3); // GPTimer

        if (_kbhit()) {
            uart0.putRx(_getch());
            irqctrl.raise_interrupt(1);
        }

        if (wasPreemtiveSwitch) {
            wasPreemtiveSwitch = false;
        } else {
            // @retvalue 0 the specified thread was not suspended. 
            //           1, the specified thread was suspended but was restarted. 
            //          >1, the specified thread is still suspended.
            tmp = ResumeThread(vecThreads[current_idx].hThread);
        }

        Sleep(100);
    }
}

