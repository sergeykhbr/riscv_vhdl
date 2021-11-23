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
#include "axi_maps.h"
#include "encoding.h"

static const int FW_IMAGE_SIZE_BYTES = 1 << 18;

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

void print_uart(const char *buf, int sz) {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    for (int i = 0; i < sz; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        uart->data = buf[i];
    }
}

void print_uart_hex(long val) {
    unsigned char t, s;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    for (int i = 0; i < 16; i++) {
        while (uart->status & UART_STATUS_TX_FULL) {}
        
        t = (unsigned char)((val >> ((15 - i) * 4)) & 0xf);
        if (t < 10) {
            s = t + '0';
        } else {
            s = (t - 10) + 'a';
        }
        uart->data = s;
    }
}

void copy_image() { 
    uint32_t tech;
    uint64_t *fwrom = (uint64_t *)ADDR_BUS0_XSLV_FWIMAGE;
    uint64_t *flash = (uint64_t *)ADDR_BUS0_XSLV_EXTFLASH;
    uint64_t *sram = (uint64_t *)ADDR_BUS0_XSLV_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;

    /** 
     * Speed-up RTL simulation by skipping coping stage.
     * Or skip this stage to avoid rewritting of externally loaded image.
     */
    tech = pnp->tech & 0xFF;

    if (tech != TECH_INFERRED && pnp->fwid == 0) {
        if (get_dip(0) == 1) {
            print_uart("Coping FLASH\r\n", 14);
            memcpy(sram, flash, FW_IMAGE_SIZE_BYTES);
        } else {
            print_uart("Coping FWIMAGE\r\n", 16);
            memcpy(sram, fwrom, FW_IMAGE_SIZE_BYTES);
        }
    }
    // Write Firmware ID to avoid copy image after soft-reset.
    pnp->fwid = 0x20211123;
}

/** This function will be used during video recording to show
 how tochange npc register value on core[1] while core[0] is running
 Zephyr OS
*/
void timestamp_output() {
    gptimers_map *tmr = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
    uint64_t start = tmr->highcnt;
    while (1) {
        if (tmr->highcnt < start || (start + SYS_HZ) < tmr->highcnt) {
            start = tmr->highcnt;
            print_uart("HIGHCNT: ", 9);
            print_uart_hex(start);
            print_uart("\r\n", 2);
        }
    }
}

void _init() {
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART1;
    gpio_map *gpio = (gpio_map *)ADDR_BUS0_XSLV_GPIO;
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_BUS0_XSLV_IRQCTRL;
  
    // mask all interrupts in interrupt controller to avoid
    // unpredictable behaviour after elf-file reloading via debug port.
    p_irq->irq_mask = 0xFFFFFFFF;

    // Half period of the uart = Fbus / 115200 / 2 = 70 MHz / 115200 / 2:
    uart->scaler = SYS_HZ / 115200 / 2;  // 40 MHz

    gpio->direction = 0xF;  // [3:0] input DIP; [11:4] output LEDs

    led_set(0x01);
    print_uart("Boot . . .", 10);
    led_set(0x02);

    copy_image();
    led_set(0x03);
    print_uart("OK\r\n", 4);

    tech = (pnp->tech >> 24) & 0xff;
    led_set(tech);
    led_set(0x04);
}

/** Not used actually */
int main() {
    while (1) {}

    return 0;
}
