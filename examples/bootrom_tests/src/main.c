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
#include "fw_api.h"

extern int test_fpu();

static const int FW_IMAGE_SIZE_BYTES = 1 << 18;

void led_set(int output) {
    // [3:0] DIP pins
    ((gpio_map *)ADDR_NASTI_SLAVE_GPIO)->ouser = (output << 4);
}

void copy_image() { 
    uint32_t tech;
    uint64_t *fwrom = (uint64_t *)ADDR_NASTI_SLAVE_FWIMAGE;
    uint64_t *sram = (uint64_t *)ADDR_NASTI_SLAVE_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;

    /** 
     * Speed-up RTL simulation by skipping coping stage.
     * Or skip this stage to avoid rewritting of externally loaded image.
     */
    tech = pnp->tech & 0xFF;

    if (tech != TECH_INFERRED && pnp->fwid == 0) {
        memcpy(sram, fwrom, FW_IMAGE_SIZE_BYTES);
    }
    // Write Firmware ID to avoid copy image after soft-reset.
    pnp->fwid = 0x20180725;
}


int main() {
    uint32_t tech;
    pnp_map *pnp = (pnp_map *)ADDR_NASTI_SLAVE_PNP;
    uart_map *uart = (uart_map *)ADDR_NASTI_SLAVE_UART1;
    gpio_map *gpio = (gpio_map *)ADDR_NASTI_SLAVE_GPIO;
    irqctrl_map *p_irq = (irqctrl_map *)ADDR_NASTI_SLAVE_IRQCTRL;

    // mask all interrupts in interrupt controller to avoid
    // unpredictable behaviour after elf-file reloading via debug port.
    p_irq->irq_mask = 0xFFFFFFFF;
    pnp->fwid = 0x20181217;
#if 1
    // Half period of the uart = Fbus / 115200 / 2 = 70 MHz / 115200 / 2:
    //uart->scaler = 304;  // 70 MHz
    //uart->scaler = 260;  // 60 MHz
    uart->scaler = 40000000 / 115200 / 2;  // 40 MHz

    gpio->direction = 0xF;  // [3:0] input DIP; [11:4] output LEDs

#else  // not finished yet
    p_irq->irq_lock = 1;
    fw_malloc_init();
    uart_isr_init();   // enable printf_uart function and Tx irq=1
    p_irq->irq_lock = 0;

    printf_uart("Tech . . .0x%08x\r\n", pnp->tech);
    printf_uart("HWID . . .0x%08x\r\n", pnp->hwid);
    printf_uart("FWID . . .0x%08x\r\n", pnp->fwid);
#endif

    led_set(0x01);
    print_uart("Boot . . .", 10);
    led_set(0x02);

    copy_image();
    led_set(0x03);
    print_uart("OK\r\n", 4);

    led_set(0x04);
#ifdef FPU_ENABLED
    print_uart("FPU. . . .", 10);
    int err = test_fpu();
    if (!err) {
        print_uart("OK\r\n", 4);
    } else {
        print_uart("FAIL\r\n", 6);
    }
#endif

    led_set(0x05);

    // jump to entry point in SRAM = 0x10000000
    //     'meps' - Machine Exception Program Coutner
    __asm__("lui t0, 0x10000");
    __asm__("csrw mepc, t0");
    __asm__("mret");

    return 0;
}
