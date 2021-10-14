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
void test_spiflash(uint64_t bar);
void test_gnss_ss(uint64_t bar);
void print_pnp(void);

int main() {
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    gpio_map *gpio = (gpio_map *)ADDR_BUS0_XSLV_GPIO;
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;
    uint64_t bar;

    if (fw_get_cpuid() != 0) {
        while (1) {}
    }

    // mask all interrupts in interrupt controller to avoid
    // unpredictable behaviour after elf-file reloading via debug port.
    p_irq->irq_mask = 0xFFFFFFFF;
    p_irq->isr_table = 0;
    pnp->fwid = 0x20190516;

    gpio->direction = 0xf;
    // !! All interrupts from PLIC are disabled (not enabled in crt.S anymore)!!!
    //li t0, 0x00000800
    //csrs mie, t0       # Enable External irq (ftom PLIC) for M mode
    p_irq->irq_lock = 1;
    fw_malloc_init();
    
    allocate_exception_table();

    uart_isr_init();   // enable printf_uart function and Tx irq=1
    p_irq->irq_lock = 0;
 
    led_set(0x01);

#if 1
    printf_uart("HARTID . . . . .%d\r\n", fw_get_cpuid());
    printf_uart("Tech . . . . . .0x%08x\r\n", pnp->tech);
    printf_uart("HWID . . . . . .0x%08x\r\n", pnp->hwid);
    printf_uart("FWID . . . . . .0x%08x\r\n", pnp->fwid);

    led_set(0x02);

    test_fpu();

    led_set(0x03);
    test_timer();      // Enabling timer[0] with 1 sec interrupts
#elif 0
    test_timer_multicycle_instructions();
#else
#endif

    led_set(0x04);
    test_missaccess();

    led_set(0x05);
    test_stackprotect();

    bar = get_dev_bar(VENDOR_GNSSSENSOR, GNSS_SUB_SYSTEM);
    led_set(0x06);
    if (bar != DEV_NONE) {
        led_set(0x07);
        test_gnss_ss(bar);
        printf_uart("GNSS_SS BAR. . .0x%08x\r\n", bar);
    }
    led_set(0x08);

    bar = get_dev_bar(VENDOR_GNSSSENSOR, GNSSSENSOR_SPI_FLASH);
    led_set(0x09);
    if (bar != DEV_NONE) {
        led_set(0x0A);
        printf_uart("SPI Flash BAR. .0x%08x\r\n", bar);
        test_spiflash(bar);
    }
    led_set(0x0B);

    led_set(0x55);
    print_pnp();

    led_set(0x1F);

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
