#include <toolchain.h>
#include <sections.h>
#include <nano_private.h>

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
#ifdef _WIN32
    LIBH_swap((uint64_t)_nanokernel.current);
#endif
    return _nanokernel.current->coopReg[COOP_REG_V0/sizeof(uint64_t)];
}

