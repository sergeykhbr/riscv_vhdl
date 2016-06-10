/**
 * @file Access methods declaration to the memory mapped devices.
 */

#ifndef _RISCV_CORE_MEMACCESS_H_
#define _RISCV_CORE_MEMACCESS_H_

#include <stdint.h>

extern uint16_t READ16(volatile uint16_t *addr);
extern uint32_t READ32(volatile uint32_t *addr);
extern uint64_t READ64(volatile uint64_t *addr);
extern void WRITE32(volatile uint32_t *addr, uint32_t val);
extern void WRITE64(volatile uint64_t *addr, uint64_t val);

#endif  // _RISCV_CORE_MEMACCESS_H_
