/*
 * Copyright (c) 2014-2015 Wind River Systems, Inc.
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

#define _ASMLANGUAGE

#include "swap_macros.h"

/**
 *
 * @brief Initiate a cooperative context switch
 *
 * The _Swap() routine is invoked by various nanokernel services to effect
 * a cooperative context switch.  Prior to invoking _Swap(), the caller
 * disables interrupts via nanoCpuIntLock() and the return 'key' is passed as a
 * parameter to _Swap(). The key is in fact the value stored in the register
 * operand of a CLRI instruction.
 *
 * It stores the intlock key parameter into current->intlock_key.

 * Given that _Swap() is called to effect a cooperative context switch,
 * the caller-saved integer registers are saved on the stack by the function
 * call preamble to _Swap(). This creates a custom stack frame that will be
 * popped when returning from _Swap(), but is not suitable for handling a return
 * from an exception. Thus, the fact that the thread is pending because of a
 * cooperative call to _Swap() has to be recorded via the _CAUSE_COOP code in
 * the relinquish_cause of the thread's tTCS. The _IrqExit()/_FirqExit() code
 * will take care of doing the right thing to restore the thread status.
 *
 * When _Swap() is invoked, we know the decision to perform a context switch or
 * not has already been taken and a context switch must happen.
 *
 * @return may contain a return value setup by a call to fiberRtnValueSet()
 *
 * C function prototype:
 *
 * unsigned int _Swap (unsigned int key);
 *
 */

.section ".text"
.globl __start
__start:
	jal _Cstart;

.section ".text"
.globl _Swap
_Swap:
	/* interrupts are locked, interrupt key is in r0 */
	la s0, _nanokernel
	ld s1,tNANO_CURRENT_OFFSET(s0)	# s1 = _nanokernel.current
	addi s2,s1,COOP_REGS_OFFSET	# s2 = &_nanokernel.current->coopRegs
	_save_context(s2)


	ld s1,tNANO_FIBER_OFFSET(s0)
	bnez s1, _swap_to_a_fiber
_swap_to_the_task:
	ld s2,tNANO_TASK_OFFSET(s0)     # current = nanokernel.task
	j _swap_current_pointer
_swap_to_a_fiber:
	ld s2,tNANO_FIBER_OFFSET(s0)
	ld s3,TCS_LINK_OFFSET(s2)	# s3 = nanokernel.fiber->link
	sd s3,tNANO_FIBER_OFFSET(s0)	# nanokernel.fiber = s3

_swap_current_pointer:
	sd s2,tNANO_CURRENT_OFFSET(s0)	# nanokernel.current = current
	addi s3,s2,COOP_REGS_OFFSET	# &s3 = &current->coopRegs
	_restore_context(s3)
	ret

.globl _IsrExit
_IsrExit:
	la s0, _nanokernel
	ld s1,tNANO_CURRENT_OFFSET(s0)	# s1 = _nanokernel.current
	ld s2,COOP_REGS_OFFSET(s1)	# s2 = _nanokernel.current->flags
	andi	s2,s2,1
	beqz	s2,_IsrExit_ending
	ld s2,tNANO_FIBER_OFFSET(s0)
	beqz	s2,_IsrExit_ending
	sd s2,tNANO_CURRENT_OFFSET(s0)	# nanokernel.current = current
	ld s3,TCS_LINK_OFFSET(s2)	# s3 = nanokernel.fiber->link
	sd s3,tNANO_FIBER_OFFSET(s0)	# nanokernel.fiber = s3

        # Set 'tp' register pointing to new task registers array, so
        # that ROM interrupt handler will restore full context on exit
	ld tp,COOP_REGS_OFFSET(s2)
_IsrExit_ending:
	ret

