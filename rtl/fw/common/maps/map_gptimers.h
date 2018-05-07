/******************************************************************************
 * @file
 * @copyright Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     General Purpose Timers register mapping definition.
******************************************************************************/
#ifndef __MAP_GPTIMERS_H__
#define __MAP_GPTIMERS_H__

#include <inttypes.h>

#define TIMER_CONTROL_ENA      (1 << 0)
#define TIMER_CONTROL_IRQ_ENA  (1 << 1)

typedef struct gptimer_type {
    volatile uint32_t control;
    volatile uint32_t rsv1;
    volatile uint64_t cur_value;
    volatile uint64_t init_value;
} gptimer_type;


typedef struct gptimers_map {
        uint64_t highcnt;
        uint32_t pending;
        uint32_t rsvr[13];
        gptimer_type timer[2];
} gptimers_map;

#endif  // __MAP_GPTIMERS_H__
