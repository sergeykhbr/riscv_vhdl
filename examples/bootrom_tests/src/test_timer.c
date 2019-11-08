/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <inttypes.h>
#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

static const char TEST_TIMER_NAME[8] = "timer";

typedef struct timer_data_type {
    int sec;
    int min;
    int hour;
} timer_data_type;

void isr_timer_empty(void) {
}

void isr_timer(void) {
    timer_data_type *p = fw_get_ram_data(TEST_TIMER_NAME);
    gptimers_map *ptmr = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
    if (++p->sec > 59) {
        p->sec = 0;
        if (++p->min > 59) {
            p->min = 0;
            if (++p->hour > 23) {
                p->hour = 0;
            }
        }
    }
    ptmr->pending = 0;
    // 1 Hz output. Interrupts once per sec.
    printf_uart("Time: %02d:%02d:%02d\r\n", p->hour, p->min, p->sec);
}

void test_timer(void) {
    timer_data_type *p;
    gptimers_map *ptmr = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;

    // Disable interrupt and timer
    ptmr->timer[0].init_value = 0;
    ptmr->timer[0].control = 0;

    fw_register_isr_handler(CFG_IRQ_GPTIMERS, isr_timer);

    p = fw_malloc(sizeof(timer_data_type));    
    fw_register_ram_data(TEST_TIMER_NAME, p);
    p->sec = 0;
    p->min = 0;
    p->hour = 0;
 
    ptmr->timer[0].init_value = SYS_HZ; // 1 s
    ptmr->timer[0].control = TIMER_CONTROL_ENA | TIMER_CONTROL_IRQ_ENA;

    fw_enable_isr(CFG_IRQ_GPTIMERS);
}

void test_timer_multicycle_instructions(void) {
    timer_data_type *p;
    gptimers_map *ptmr = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
    uint64_t endtime = ptmr->highcnt + SYS_HZ;
    double a = 1.1;

    // Disable interrupt and timer
    ptmr->timer[0].init_value = 0;
    ptmr->timer[0].control = 0;

    fw_register_isr_handler(CFG_IRQ_GPTIMERS, isr_timer_empty);

    ptmr->timer[0].init_value = 1000; 
    ptmr->timer[0].control = TIMER_CONTROL_ENA | TIMER_CONTROL_IRQ_ENA;

    fw_enable_isr(CFG_IRQ_GPTIMERS);
//    while (ptmr->highcnt < endtime) {
    while (1) {
        a = a * 1.1;
    }
}

