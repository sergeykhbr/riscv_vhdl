/*
 * Copyright (c) 2013-2014 Wind River Systems, Inc.
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

/**
 * @file
 * @brief Private nanokernel definitions (RISC-V)
 *
 * This file contains private nanokernel structures definitions and various
 * other definitions for the ARM Cortex-M3 processor architecture.
 *
 * This file is also included by assembly language files which must #define
 * _ASMLANGUAGE before including this header file.  Note that nanokernel
 * assembly source files obtains structure offset values via "absolute symbols"
 * in the offsets.o module.
 */

#ifndef _NANO_PRIVATE_H
#define _NANO_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <toolchain.h>
#include <sections.h>
#include <irq.h>
#include <nanokernel.h>		   /* public nanokernel API */
#include <../../../kernel/nanokernel/include/nano_internal.h>
#include <stdint.h>
#include <misc/dlist.h>

#include <misc/util.h>
#include "../core/swap_macros.h"

#ifdef _DEBUG
#include <misc/printk.h>
#define PRINTK(...) printk(__VA_ARGS__)
#else
#define PRINTK(...)
#endif /* DEBUG */


/* increase to 16 bytes (or more?) to support SSE/SSE2 instructions? */

/* stacks */

#define STACK_ALIGN_SIZE 4

#define STACK_ROUND_UP(x) ROUND_UP(x, STACK_ALIGN_SIZE)
#define STACK_ROUND_DOWN(x) ROUND_DOWN(x, STACK_ALIGN_SIZE)

/*
 * Reason a thread has relinquished control: fibers can only be in the NONE
 * or COOP state, tasks can be one in the four.
 */
#define _CAUSE_NONE 0
#define _CAUSE_COOP 1
#define _CAUSE_RIRQ 2
#define _CAUSE_FIRQ 3


/*
 * Bitmask definitions for the struct tcs->flags bit field
 *
 * The USE_FP flag bit will be set whenever a thread uses any non-integer
 * capability, whether it's just the x87 FPU capability, SSE instructions, or
 * a combination of both. The USE_SSE flag bit will only be set if a thread
 * uses SSE instructions.
 *
 * Note: Any change to the definitions USE_FP and USE_SSE must also be made to
 * nanokernel/x86/arch.h.
 */

#define FIBER 0
#define TASK 0x1	       /* 1 = task, 0 = fiber   */
#define INT_ACTIVE 0x2     /* 1 = executing context is interrupt handler */
#define EXC_ACTIVE 0x4     /* 1 = executing context is exception handler */
#define USE_FP 0x10	       /* 1 = thread uses floating point unit */
#define USE_SSE 0x20       /* 1 = thread uses SSEx instructions */
#define PREEMPTIBLE 0x100  /* 1 = preemptible thread */
#define ESSENTIAL 0x200    /* 1 = system thread that must not abort */
#define NO_METRICS 0x400   /* 1 = _Swap() not to update task metrics */
#define NO_METRICS_BIT_OFFSET 0xa /* Bit position of NO_METRICS */

#define INT_OR_EXC_MASK (INT_ACTIVE | EXC_ACTIVE)


/*
 * The following structure defines the set of 'non-volatile' integer registers.
 * These registers must be preserved by a called C function.  These are the
 * only registers that need to be saved/restored when a cooperative context
 * switch occurs.
 *
 *      x0      zero    Hard-wired zero             -
 *      x1      ra      Return address              Caller
 *      x2      s0/fp   Saved register/frame pointer Callee
 *      x3-13   s1-11   Saved registers             Callee
 *      x14     sp      Stack pointer               Callee
 *      x15     tp      Thread pointer              Callee
 *      x16-17  v0-1    Return values               Caller
 *      x18-25  a0-7    Function arguments          Caller
 *      x26-30  t0-4    Temporaries                 Caller
 *      x31     gp      Global pointer              -
 *      -------------
 *      f0-15   fs0-15  FP saved registers          Callee
 *      f16-17  fv0-1   FP return values            Caller
 *      f18-25  fa0-7   FP arguments                Caller
 *      f26-31  ft0-5   FP temporaries              Caller * 
 */

/*
 * The following structure defines the set of 'volatile' integer registers.
 * These registers need not be preserved by a called C function.  Given that
 * they are not preserved across function calls, they must be save/restored
 * (along with the s_coop_reg) when a preemptive context switch occurs.
 */

typedef struct s_preempReg {

	/*
	 * The volatile registers 'eax', 'ecx' and 'edx' area not included in
	 *the
	 * definition of 'tPreempReg' since the interrupt stubs
	 *(_IntEnt/_IntExit)
	 * and exception stubs (_ExcEnt/_ExcEnter) use the stack to save and
	 * restore the values of these registers in order to support interrupt
	 * nesting.  The stubs do _not_ copy the saved values from the stack
	 *into
	 * the TCS.
	 *
	 * unsigned long eax;
	 * unsigned long ecx;
	 * unsigned long edx;
	 */
     int empty;

} tPreempReg;



/*
 * The thread control stucture definition.  It contains the
 * various fields to manage a _single_ thread. The TCS will be aligned
 * to the appropriate architecture specific boundary via the
 * _new_thread() call.
 */

struct tcs {
	/*
	 * Link to next thread in singly-linked thread list (such as
	 * prioritized list of runnable fibers, or list of fibers waiting on a
	 * nanokernel FIFO).
	 */
	struct tcs *link;

	/*
	 * See the above flag definitions above for valid bit settings.  This
	 * field must remain near the start of struct tcs, specifically
	 * before any #ifdef'ed fields since the host tools currently use a
	 * fixed
	 * offset to read the 'flags' field.
	 */
	uint64_t flags;

	/*
	 * Storage space for integer registers.  These must also remain near
	 * the start of struct tcs for the same reason mention for
	 * 'flags'.
	 */
    uint64_t coopReg[COOP_REGS_TOTAL];     /* non-volatile integer register storage */
	tPreempReg preempReg; /* volatile integer register storage */

#if defined(CONFIG_THREAD_MONITOR)
	struct tcs *next_thread; /* next item in list of ALL fiber+tasks */
#endif
#ifdef CONFIG_GDB_INFO
	void *esfPtr; /* pointer to exception stack frame saved by */
		      /* outermost exception wrapper */
#endif		      /* CONFIG_GDB_INFO */
    /*
     * thread priority used to sort linked list
     *         fiber priority, -1 for a task
     */
	int prio;

#if (defined(CONFIG_FP_SHARING) || defined(CONFIG_GDB_INFO))
	/*
	 * Nested exception count to maintain setting of EXC_ACTIVE flag across
	 * outermost exception.  EXC_ACTIVE is used by _Swap() lazy FP
	 * save/restore and by debug tools.
	 */
	unsigned excNestCount; /* nested exception count */
#endif /* CONFIG_FP_SHARING || CONFIG_GDB_INFO */

#ifdef CONFIG_THREAD_CUSTOM_DATA
	void *custom_data;     /* available for custom use */
#endif

#ifdef CONFIG_NANO_TIMEOUTS
	struct _nano_timeout nano_timeout;
#endif

#ifdef CONFIG_ERRNO
	int errno_var;
#endif
};

/*
 * The nanokernel structure definition.  It contains various fields to
 * manage _all_ the threads in the nanokernel (system level).
 */

typedef struct s_NANO {
	struct tcs *fiber;   /* singly linked list of runnable fibers */
	struct tcs *task;    /* pointer to runnable task */
	struct tcs *current; /* currently scheduled thread (fiber or task) */
#if defined(CONFIG_THREAD_MONITOR)
	struct tcs *threads; /* singly linked list of ALL fiber+tasks */
#endif
	unsigned nested;  /* nested interrupt count */
	char *common_isp; /* interrupt stack pointer base */

#ifdef CONFIG_ADVANCED_POWER_MANAGEMENT
	int32_t idle; /* Number of ticks for kernel idling */
#endif		      /* CONFIG_ADVANCED_POWER_MANAGEMENT */


#ifdef CONFIG_FP_SHARING
	/*
	 * A 'current_sse' field does not exist in addition to the 'current_fp'
	 * field since it's not possible to divide the IA-32 non-integer
	 * registers into 2 distinct blocks owned by differing threads.  In
	 * other words, given that the 'fxnsave/fxrstor' instructions
	 * save/restore both the X87 FPU and XMM registers, it's not possible
	 * for a thread to only "own" the XMM registers.
	 */

	struct tcs *current_fp; /* thread (fiber or task) that owns the FP regs */
#endif			  /* CONFIG_FP_SHARING */
#ifdef CONFIG_NANO_TIMEOUTS
	sys_dlist_t timeout_q;
	int32_t task_timeout;
#endif
} tNANO;

/*
 * There is only a single instance of the s_NANO structure, given that there
 * is only a single nanokernel in the system: _nanokernel
 */

extern tNANO _nanokernel;

/* INLINE function definitions */

/**
 *
 * @brief Performs architecture-specific initialization
 *
 * This routine performs architecture-specific initialization of the nanokernel.
 * Trivial stuff is done inline; more complex initialization is done via
 * function calls.
 *
 * @return N/A
 */
static INLINE void nanoArchInit(void)
{

}

/**
 *
 * @brief Set the return value for the specified fiber (inline)
 *
 * @param fiber pointer to fiber
 * @param value value to set as return value
 *
 * The register used to store the return value from a function call invocation
 * is set to <value>.  It is assumed that the specified <fiber> is pending, and
 * thus the fibers context is stored in its TCS.
 *
 * @return N/A
 */
static INLINE void fiberRtnValueSet(struct tcs *fiber, uint64_t value)
{
	fiber->coopReg[COOP_REG_V0/sizeof(uint64_t)] = value;
}

/* definitions to support nanoCpuExcConnect() */

#define _EXC_STUB_SIZE 0x14

/* function prototypes */

extern void nano_cpu_atomic_idle(unsigned int imask);

extern void nanoCpuExcConnect(unsigned int vector,
			      void (*routine)(NANO_ESF *pEsf));

extern void _IntVecSet(unsigned int vector,
		       void (*routine)(void *),
		       unsigned int dpl);


/*
 * _IntLibInit() is called from the non-arch specific nanokernel function,
 * _nano_init(). The IA-32 nanokernel does not require any special
 * initialization of the interrupt subsystem. However, we still need to
 * provide an _IntLibInit() of some sort to prevent build errors.
 */
static INLINE void _IntLibInit(void)
{
}

int _stub_alloc(unsigned int *ep, unsigned int limit);
void *_get_dynamic_stub(int stub_idx, void *base_ptr);
uint8_t _stub_idx_from_vector(int vector);

/* the _idt_base_address symbol is generated via a linker script */
extern unsigned char _idt_base_address[];

#include <stddef.h> /* For size_t */


#ifdef __cplusplus
}
#endif

#define _IS_IN_ISR() (_nanokernel.nested != 0)

#endif /* _NANO_PRIVATE_H */
