#
# Copyright (c) 2014-2015 Wind River Systems, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#include "swap_macros.h"

# @brief Initiate a cooperative context switch
#
# The _Swap() routine is invoked by various nanokernel services to effect
# a cooperative context switch.  Prior to invoking _Swap(), the caller
# disables interrupts via nanoCpuIntLock() and the return 'key' is passed as a
# parameter to _Swap(). The key is in fact the value stored in the register
# operand of a CLRI instruction.
#
# It stores the intlock key parameter into current->intlock_key.
#
# Given that _Swap() is called to effect a cooperative context switch,
# the caller-saved integer registers are saved on the stack by the function
# call preamble to _Swap(). This creates a custom stack frame that will be
# popped when returning from _Swap(), but is not suitable for handling a return
# from an exception. Thus, the fact that the thread is pending because of a
# cooperative call to _Swap() has to be recorded via the _CAUSE_COOP code in
# the relinquish_cause of the thread's tTCS. The _IrqExit()/_FirqExit() code
# will take care of doing the right thing to restore the thread status.
#
# When _Swap() is invoked, we know the decision to perform a context switch or
# not has already been taken and a context switch must happen.
#
# @return may contain a return value setup by a call to fiberRtnValueSet()
#
# C function prototype:
#
# unsigned int _Swap (unsigned int key);
#

.section ".text"
.globl _Swap
_Swap:
	/* interrupts are locked, interrupt key is in r0 */

  	fence
	la t1, _nanokernel
        # TODO: 
	eret
