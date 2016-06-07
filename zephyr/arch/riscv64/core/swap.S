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
	_save_context(tp)
        sd ra,COOP_REG_MEPC(tp)
	la s0, _nanokernel
	ld s1,tNANO_CURRENT_OFFSET(s0)	# s1 = _nanokernel.current
	sw x0,TCS_INTLOCK_OFFSET(s1)    # _nanokernel.current->intlock = 0

	ld s1,tNANO_FIBER_OFFSET(s0)
	bnez s1, _swap_to_a_fiber
_swap_to_the_task:
	ld s2,tNANO_TASK_OFFSET(s0)     # current = nanokernel.task
	sd s2,tNANO_CURRENT_OFFSET(s0)	# nanokernel.current = current
        lw a0,TCS_INTLOCK_OFFSET(s2)
        jal _arch_irq_unlock            # _arch_irq_unlock(current->intlock)
	addi tp,s2,TCS_COOP_REGS_OFFSET	# tp = &current->coopRegs
	_restore_context(tp)
        ld t6,COOP_REG_MEPC(tp)
	jalr x0,t6

_swap_to_a_fiber:
	ld s2,tNANO_FIBER_OFFSET(s0)
	ld s3,TCS_LINK_OFFSET(s2)	# s3 = nanokernel.fiber->link
	sd s3,tNANO_FIBER_OFFSET(s0)	# nanokernel.fiber = s3
	sd s2,tNANO_CURRENT_OFFSET(s0)	# nanokernel.current = current
        lw a0,TCS_INTLOCK_OFFSET(s2)
        jal _arch_irq_unlock            # _arch_irq_unlock(current->intlock)
	addi tp,s2,TCS_COOP_REGS_OFFSET	# tp = &current->coopRegs
	_restore_context(tp)
	ret

.globl _IsrExit
_IsrExit:
        addi sp,sp,40
        sd s0,0(sp)
        sd s2,8(sp)
        sd s3,16(sp)

	la s0, _nanokernel
	ld s1,tNANO_CURRENT_OFFSET(s0)	# s1 = _nanokernel.current
	ld s2,TCS_FLAGS_OFFSET(s1)	# s2 = _nanokernel.current->flags
        # Only Task may be preemptedly switched (Fibers can not)
        andi s2,s2,1
	beqz s2,_IsrExit_ending         # ((s2 & TASK) != TASK) goto _IsrExit_ending
	ld s2,tNANO_FIBER_OFFSET(s0)
	beqz s2,_IsrExit_ending         # if(_nanokernel.current->fiber==0) goto _IsrExit_ending
        
        sd ra,24(sp)
        sd a0,32(sp)
        jal _arch_irq_lock_state        # a0 = _arch_irq_lock_state()
        sw a0,TCS_INTLOCK_OFFSET(s1)	# _nanokernel.current->intlock=_arch_irq_lock_state()
	sd s2,tNANO_CURRENT_OFFSET(s0)	# nanokernel.current = current
	ld s3,TCS_LINK_OFFSET(s2)	# s3 = nanokernel.fiber->link
	sd s3,tNANO_FIBER_OFFSET(s0)	# nanokernel.fiber = s3

        lw a0,TCS_INTLOCK_OFFSET(s2)
        jal _arch_irq_unlock            # _arch_irq_unlock(current->intlock)

        # Set 'tp' register pointing to new task registers array, so
        # that ROM interrupt handler will restore full context on exit
	addi tp,s2,TCS_COOP_REGS_OFFSET
        ld a0,32(sp)
        ld ra,24(sp)
_IsrExit_ending:
        ld s0,0(sp)
        ld s2,8(sp)
        ld s3,16(sp)
        addi sp,sp,-40
	ret
