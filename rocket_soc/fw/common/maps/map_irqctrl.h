/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     Interrupt Controller register mapping definition.
******************************************************************************/

#ifndef __MAP_IRQCTRL_H__
#define __MAP_IRQCTRL_H__

#include <inttypes.h>

typedef void (*IRQ_HANDLER)(void *arg);

typedef struct irqctrl_map {
    volatile uint32_t irq_mask;     // 0x00: [RW] 1=disable; 0=enable
    volatile uint32_t irq_pending;  // 0x04: [RW]
    volatile uint32_t irq_clear;    // 0x08: [WO]
    volatile uint32_t irq_rise;     // 0x0C: [WO]
    volatile uint64_t isr_table;    // 0x10: [RW]
    volatile uint64_t dbg_cause;    // 0x18:
    volatile uint64_t dbg_epc;      // 0x20:
    volatile uint32_t irq_lock;     // 0x28: lock/unlock all interrupts
    volatile uint32_t irq_cause_idx;// 0x2c:
} irqctrl_map;

#endif  // __MAP_IRQCTRL_H__
