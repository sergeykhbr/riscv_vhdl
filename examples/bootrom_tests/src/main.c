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

void isr_uart0_tx();

void allocate_exception_table(void);
void allocate_interrupt_table(void);
void test_l2coherence(void);
void test_plic(void);
void test_fpu(void);
void test_swirq(void);
void test_mtimer(void);
void test_missaccess(void);
void test_stackprotect(void);
void test_spiflash(uint64_t bar);
void test_gnss_ss(uint64_t bar);
int test_pmp();
int test_mmu();
void print_pnp(void);
int hwthread1(void);
int hwthread2(void);
int hwthread3(void);


int main() {
    uint32_t cfg;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    gpio_map *gpio = (gpio_map *)ADDR_BUS0_XSLV_GPIO;
    uint64_t bar;
    uint32_t cpu_max;

    switch (fw_get_cpuid()) {
    case 0:
        break;
    case 1:
        hwthread1();
        break;
    case 2:
        hwthread2();
        break;
    case 3:
        hwthread3();
        break;
    default:
        while (1) {}
    }

    pnp->fwid = 0x20190516;
    gpio->input_en = 0x000f;
    gpio->output_en = 0xfff0;
    fw_malloc_init();
    
    allocate_exception_table();
    allocate_interrupt_table();

    uart_isr_init();   // enable printf_uart function and Tx irq=1
 
    led_set(0x01);

    cpu_max = pnp->cfg >> 28;

    printf_uart("HARTID . . . . .%d\r\n", fw_get_cpuid());
    printf_uart("HARTS. . . . . .%d\r\n", cpu_max);
    printf_uart("PLIC_IRQS  . . .%d\r\n", (pnp->cfg & 0xFF));
    printf_uart("HWID . . . . . .0x%08x\r\n", pnp->hwid);
    printf_uart("FWID . . . . . .0x%08x\r\n", pnp->fwid);

    led_set(0x02);

    test_plic();
    test_mtimer();
    test_swirq();
    test_l2coherence();
    test_pmp();
    test_mmu();

    led_set(0x03);
    test_fpu();

    led_set(0x04);
    test_missaccess();

    led_set(0x05);
    test_stackprotect();

    bar = get_dev_bar(pnp, VENDOR_GNSSSENSOR, GNSS_SUB_SYSTEM);
    led_set(0x06);
    if (bar != DEV_NONE) {
        led_set(0x07);
        test_gnss_ss(bar);
        printf_uart("GNSS_SS BAR. . .0x%016llx\r\n", bar);
    }
    led_set(0x08);

    bar = get_dev_bar(pnp, VENDOR_GNSSSENSOR, GNSSSENSOR_SPI_FLASH);
    led_set(0x09);
    if (bar != DEV_NONE) {
        led_set(0x0A);
        printf_uart("SPI Flash BAR. .0x%08x\r\n", bar);
    }
    led_set(0x0B);

    led_set(0x55);
    print_pnp();

    led_set(0x1F);

    // TODO: implement test console
    while (1) {
        // Check DDR init done
        if (pnp->rsrv1 & 1) {
             uint64_t *ddr = (uint64_t *)0x80000000ull;
             print_uart("DDR Init . .DONE\r\n", 18);
             ddr[0] = 0x1122334455667788ull;
             ddr[1] = 0xffeeddccbbaa9988ull;
             ddr[1*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[2*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[3*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[4*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[5*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[6*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[7*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[8*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[9*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[10*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[11*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[12*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[13*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[14*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[15*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[16*1024*1024] = 0xabcdabcdaabbccddull;
             ddr[17*1024*1024] = 0xabcdabcdaabbccddull;
             printf_uart("DDR[0] . .0x%016llx\r\n", ddr[0]);
             printf_uart("DDR[1] . .0x%016llx\r\n", ddr[1]);
             printf_uart("DDR[1MB] . .0x%016llx\r\n", ddr[1024*1024]);
        }

        // temporary put it here, while PLIC not fully ready
        isr_uart0_tx();
    }

    // NEVER REACH THIS POINT

    // jump to entry point in SRAM = 0x10000000
    //     'meps' - Machine Exception Program Coutner
    __asm__("lui t0, 0x10000");
    __asm__("csrw mepc, t0");
    __asm__("mret");

    return 0;
}
