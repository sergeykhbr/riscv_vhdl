/*
 * Copyright (c) 2010-2015 Wind River Systems, Inc.
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
 * @brief Nanokernel thread support primitives
 *
 * This module provides core nanokernel fiber related primitives for the IA-32
 * processor architecture.
 */

#ifdef CONFIG_MICROKERNEL
#include <microkernel.h>
#include <micro_private_types.h>
#endif /* CONFIG_MICROKERNEL */
#ifdef CONFIG_INIT_STACKS
#include <string.h>
#endif /* CONFIG_INIT_STACKS */

#include <toolchain.h>
#include <sections.h>
#include <nano_private.h>
#include <wait_q.h>

#ifdef _WIN32
extern int LIBH_create_thread(char *pStackMem,
                              unsigned stackSize, 
                              int priority, 
                              unsigned options);
#endif

/* the one and only nanokernel control structure */

tNANO _nanokernel = {0};

/**
 *
 * @brief Create a new kernel execution thread
 *
 * This function is utilized to create execution threads for both fiber
 * threads and kernel tasks.
 *
 * The "thread control block" (TCS) is carved from the "end" of the specified
 * thread stack memory.
 *
 * @param pStackmem the pointer to aligned stack memory
 * @param stackSize the stack size in bytes
 * @param pEntry thread entry point routine
 * @param parameter1 first param to entry point
 * @param parameter2 second param to entry point
 * @param parameter3 third param to entry point
 * @param priority  thread priority
 * @param options thread options: USE_FP, USE_SSE
 *
 *
 * @return opaque pointer to initialized TCS structure
 */
void _new_thread(char *pStackMem, unsigned stackSize,
		 void *uk_task_ptr, _thread_entry_t pEntry,
		 void *parameter1, void *parameter2, void *parameter3,
		 int priority, unsigned options)
{
	/* ptr to the new task's tcs */
	struct tcs *tcs = (struct tcs *)pStackMem;
	unsigned long *pInitialCtx;

#ifdef CONFIG_INIT_STACKS
	memset(pStackMem, 0xaa, stackSize);
#endif

	/* carve the thread entry struct from the "base" of the stack */

	pInitialCtx =
		(unsigned long *)STACK_ROUND_DOWN(pStackMem + stackSize);

	/*
	 * Create an initial context on the stack expected by the _Swap()
	 * primitive.
	 * Given that both task and fibers execute at privilege 0, the
	 * setup for both threads are equivalent.
	 */

	/* push arguments required by _thread_entry() */

	*--pInitialCtx = (unsigned long)parameter3;
	*--pInitialCtx = (unsigned long)parameter2;
	*--pInitialCtx = (unsigned long)parameter1;
	*--pInitialCtx = (unsigned long)pEntry;
	*--pInitialCtx = (unsigned long)_thread_entry;

	tcs->link = (struct tcs *)NULL; /* thread not inserted into list yet */
	tcs->prio = priority;
	if (priority == -1) {
		tcs->flags = PREEMPTIBLE | TASK;
	} else {
		tcs->flags = FIBER;
    }

#ifdef CONFIG_THREAD_CUSTOM_DATA
	/* Initialize custom data field (value is opaque to kernel) */
	tcs->custom_data = NULL;
#endif

#if defined(CONFIG_THREAD_MONITOR)
	/*
	 * In debug mode tcs->entry give direct access to the thread entry
	 * and the corresponding parameters.
	 */
	tcs->entry = (struct __thread_entry *)(pInitCtx);
#endif /* CONFIG_THREAD_MONITOR */
#ifdef CONFIG_MICROKERNEL
	tcs->uk_task_ptr = uk_task_ptr;
#else
	ARG_UNUSED(uk_task_ptr);
#endif

	_nano_timeout_tcs_init(tcs);

	/*
	 * @note Stack area for 'callee' saved registers: s0-s11, sp,tp can be
	 * left uninitialized, since _thread_entry() doesn't care about the values
	 * of these registers when it begins execution
	 */
    pInitialCtx -= COOP_STACKFRAME_SIZE;

    *(uint64_t *)((uint8_t *)(tcs) + COOP_REG_SP) = (unsigned long)pInitialCtx;
	PRINTK("\nInitial context SP = 0x%x\n", (unsigned long)pInitialCtx);
    tcs->coopReg[COOP_REG_RA/sizeof(uint64_t)] = (uint64_t)_thread_entry;
    tcs->coopReg[COOP_REG_A0/sizeof(uint64_t)] = (uint64_t)pEntry;
    tcs->coopReg[COOP_REG_A1/sizeof(uint64_t)] = (uint64_t)parameter1;
    tcs->coopReg[COOP_REG_A2/sizeof(uint64_t)] = (uint64_t)parameter2;
    tcs->coopReg[COOP_REG_A3/sizeof(uint64_t)] = (uint64_t)parameter3;
    tcs->coopReg[COOP_REG_SP/sizeof(uint64_t)] = 
        (uint64_t)(STACK_ROUND_DOWN(pStackMem + stackSize)) - COOP_REGS_TOTAL;
#ifdef _WIN32
    LIBH_create_thread(pStackMem, stackSize, priority, options);
#endif
}
