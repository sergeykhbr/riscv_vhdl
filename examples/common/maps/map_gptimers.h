/******************************************************************************
 * @file
 * @copyright Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     General Purpose Timers register mapping definition.
******************************************************************************/
#ifndef __MAP_GPTIMERS_H__
#define __MAP_GPTIMERS_H__

#include <inttypes.h>

#define TIMER_CONTROL_ENA           (1 << 0)
#define TIMER_CONTROL_IRQ_ENA       (1 << 1)
#define TIMER_CONTROL_PWM_ENA       (1 << 4)
#define TIMER_CONTROL_PWM_POLARITY  (1 << 5)

typedef struct gptimer_type {
    volatile uint32_t control;
    volatile uint32_t rsv1;
    volatile uint64_t cur_value;
    volatile uint64_t init_value;
    volatile uint64_t pwm_threshold;
} gptimer_type;


typedef struct gptimers_map {
    volatile uint64_t highcnt;
    volatile uint32_t pending;
    volatile uint32_t pwm;
    uint32_t rsvr[12];
    gptimer_type timer[2];
} gptimers_map;

#endif  // __MAP_GPTIMERS_H__
