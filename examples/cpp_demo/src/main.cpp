/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <string.h>
#include <cstdio>
#include "axi_maps.h"
#include "encoding.h"
#include "general.h"

extern "C" {

static int foo_cnt = 0;

class Foo {
 public:
    Foo(const char *name) : idx_(0) {
        printf("Foo '%s', cnt = %d\n", name, ++foo_cnt);
    }

 private:
    int idx_;
};

int fw_get_cpuid() {
    int ret;
    asm("csrr %0, mhartid" : "=r" (ret));
    return ret;
}

void led_set(int output) {
    // [3:0] DIP pins
    ((gpio_map *)ADDR_BUS0_XSLV_GPIO)->ouser = (output << 4);
}

int get_dip(int idx) {
    // [3:0] DIP pins
    int dip = ((gpio_map *)ADDR_BUS0_XSLV_GPIO)->iuser >> idx;
    return dip & 1;
}


/** This function will be used during video recording to show
 how tochange npc register value on core[1] while core[0] is running
 Zephyr OS
*/
void timestamp_output() {
    gptimers_map *tmr = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
    uint64_t start = tmr->highcnt;
    uint32_t *sram = (uint32_t *)ADDR_BUS0_XSLV_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    uint64_t tdata;
    while (1) {
#if 0
        // Just do something
        for (int i = 0; i < 16; i++) {
            tdata = sram[16*1024 + i];
            //sram[16*1024] = tdata;
            pnp->fwdbg1 = tdata;
        }
        if (tmr->highcnt < start || (start + SYS_HZ) < tmr->highcnt) {
            start = tmr->highcnt;
            print_uart("HIGHCNT: ", 9);
            print_uart_hex(start);
            print_uart("\r\n", 2);
        }
        asm("fence.i");  // flush L2 either
#endif
    }
}

void _init() {
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    gpio_map *gpio = (gpio_map *)ADDR_BUS0_XSLV_GPIO;
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;

    if (fw_get_cpuid() != 0) {
        timestamp_output();
    }

    // mask all interrupts in interrupt controller to avoid
    // unpredictable behaviour after elf-file reloading via debug port.
    p_irq->irq_mask = 0xFFFFFFFF;

    // Half period of the uart = Fbus / 115200 / 2 = 70 MHz / 115200 / 2:
    uart->scaler = SYS_HZ / 115200 / 2;  // 40 MHz

    gpio->direction = 0xF;  // [3:0] input DIP; [11:4] output LEDs


    led_set(0x01);
    printf("%s", "Init . . .done\n");
    led_set(0x02);
    Foo A("A");
    Foo B("B");
}

/** Not used actually */
int main() {
    while (1) {}

    return 0;
}

}  // extern "C"
