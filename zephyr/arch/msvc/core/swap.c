#include <toolchain.h>
#include <sections.h>
#include <nano_private.h>

//
unsigned int _Swap(unsigned int fl) {
    tNANO *r1 = &_nanokernel;
    struct tcs *r2 = r1->current;

    r2 = r1->fiber;
    if (r2 == 0) {
        //_swap_to_the_task
        r2 = _nanokernel.task;
    } else {
        //_swap_to_a_fiber:
        struct tcs *r3 = r2->link;
        r1->fiber = r3;
    }

    _nanokernel.current = r2;
    
#ifdef _WIN32
    _nanokernel.current->return_value = LIBH_swap((uint64_t)_nanokernel.current);
#endif
    return (unsigned int)_nanokernel.current->return_value;
}

