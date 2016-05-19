#include <toolchain.h>
#include <sections.h>
#include <nano_private.h>

void _save_registers(uint64_t *to) {
}

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

    _nanokernel.current = current;
#ifdef _WIN32
    _nanokernel.current->return_value = LIBH_swap((uint64_t)_nanokernel.current);
#endif
    return _nanokernel.current->return_value;
}

