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

#include <toolchain.h>
#include <sections.h>
#include <nano_private.h>

extern void LIBH_swap(uint64_t tc_addr);
extern void LIBH_swap_preemptive(uint64_t tc_addr);

//
uint64_t _Swap(uint64_t fl) {
    struct tcs *current;
    if (_nanokernel.fiber == 0) {
        //_swap_to_the_task
        current   = _nanokernel.task;
    } else {
        //_swap_to_a_fiber:
        current   = _nanokernel.fiber;
        _nanokernel.fiber = _nanokernel.fiber->link;
    }

    _nanokernel.current->intlock = 0;   // Cooperative switch with automatic unlocking
    _nanokernel.current = current;
    irq_unlock(_nanokernel.current->intlock);
    LIBH_swap((uint64_t)_nanokernel.current);
    return _nanokernel.current->coopReg[COOP_REG_A0/sizeof(uint64_t)];
}

void _IsrExit(void) {
    if ((_nanokernel.current->flags & TASK) == TASK) {
        if (_nanokernel.fiber) {
            _nanokernel.current->intlock = _arch_irq_lock_state();
            _nanokernel.current = _nanokernel.fiber;
            _nanokernel.fiber = _nanokernel.fiber->link;

            irq_unlock(_nanokernel.current->intlock);
            LIBH_swap_preemptive((uint64_t)_nanokernel.current);
        }
    }
}


