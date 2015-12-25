/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     Interrupt Controller register mapping definition.
******************************************************************************/

#ifndef __MAP_IRQCTRL_H__
#define __MAP_IRQCTRL_H__

#include <inttypes.h>

typedef void (*IRQ_HANDLER)(void);

typedef struct irqctrl_map {
    volatile uint32_t irq_mask;    //[RW] 1=disable; 0=enable
    volatile uint32_t irq_pending; //[RW]
    volatile uint32_t irq_clear;   //[WO]
    volatile uint32_t irq_rise;    //[WO]
    volatile uint64_t irq_handler; //[RW]
    volatile uint64_t dbg_cause;
    volatile uint64_t dbg_epc;
} irqctrl_map;

#endif  // __MAP_IRQCTRL_H__
