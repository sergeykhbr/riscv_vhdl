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

#include <string.h>
#include "axi_maps.h"
#include "encoding.h"
#include "fw_api.h"

void allocate_exception_table(void);
void test_fpu(void);
void test_timer(void);
void test_timer_multicycle_instructions(void);
void test_missaccess(void);
void test_stackprotect(void);
void print_pnp(void);

int main() {
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    gpio_map *gpio = (gpio_map *)ADDR_NASTI_SLAVE_GPIO;
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_NASTI_SLAVE_IRQCTRL;

    if (fw_get_cpuid() != 0) {
        while (1) {}
    }

    // mask all interrupts in interrupt controller to avoid
    // unpredictable behaviour after elf-file reloading via debug port.
    p_irq->irq_mask = 0xFFFFFFFF;
    pnp->fwid = 0x20190516;

    led_set(0x01);

    p_irq->irq_lock = 1;
    fw_malloc_init();
    
    allocate_exception_table();

    uart_isr_init();   // enable printf_uart function and Tx irq=1
    p_irq->irq_lock = 0;

#if 1
    printf_uart("HARTID . . . . .%d\r\n", fw_get_cpuid());
    printf_uart("Tech . . . . . .0x%08x\r\n", pnp->tech);
    printf_uart("HWID . . . . . .0x%08x\r\n", pnp->hwid);
    printf_uart("FWID . . . . . .0x%08x\r\n", pnp->fwid);

    led_set(0x02);

    test_fpu();

    led_set(0x03);
    test_timer();      // Enabling timer[0] with 1 sec interrupts
#else
    test_timer_multicycle_instructions();
#endif

    led_set(0x04);
    test_missaccess();

    led_set(0x05);
    test_stackprotect();

    led_set(0x06);
    print_pnp();

    led_set(0x07);

    // TODO: implement test console
    while (1) {}

    // NEVER REACH THIS POINT

    // jump to entry point in SRAM = 0x10000000
    //     'meps' - Machine Exception Program Coutner
    __asm__("lui t0, 0x10000");
    __asm__("csrw mepc, t0");
    __asm__("mret");

    return 0;
}
