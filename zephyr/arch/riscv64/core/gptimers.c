#include <toolchain.h>
#include <sections.h>
#include <nano_private.h>

//
uint32_t _do_read_cpu_timestamp32(void) {
    return 500;//READ32();
}

