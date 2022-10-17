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
#include <axi_maps.h>
#include <encoding.h>
#include "sd_uefi.h"
#include "uart.h"

static const int FW_IMAGE_SIZE_BYTES = 1 << 18;

int fw_get_cpuid() {
    int ret;
    asm("csrr %0, mhartid" : "=r" (ret));
    return ret;
}

void led_set(int output) {
    // [3:0] DIP pins
    ((gpio_map *)ADDR_BUS0_XSLV_GPIO)->output_val = (output << 4);
}

int get_dip(int idx) {
    // [3:0] DIP pins
    int dip = ((gpio_map *)ADDR_BUS0_XSLV_GPIO)->input_val >> idx;
    return dip & 1;
}

int get_dips() {
    // [3:0] DIP pins
    int dip = ((gpio_map *)ADDR_BUS0_XSLV_GPIO)->input_val;
    return dip & 0xF;
}



void copy_image() { 
    uint64_t *fwrom = (uint64_t *)ADDR_BUS0_XSLV_FWIMAGE;
    uint64_t *sram = (uint64_t *)ADDR_BUS0_XSLV_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;

    uint64_t qspi2 = ~0ull;
    printf_uart("dip=%02x\r\n", get_dips());
    if (get_dips() == 0xB) {
        // Load from SD-card
        qspi2 = get_dev_bar(pnp, VENDOR_GNSSSENSOR, GNSSSENSOR_SPI_FLASH);
        if (qspi2 != ~0ull) {
            print_uart("Select . .QSPI2\r\n", 17);
            if (run_from_sdcard() == -1) {
                print_uart("QSPI2. . .FAILED\r\n", 18);
                qspi2 = ~0ull;
            }
        }
    }

    if (qspi2 != ~0ull) {
        // Copy BSL from SD-card
    } else if (pnp->fwid == 0) {
        // Check if SRAM already initialized in RTL simulation, then skip copying
        uint64_t *dst = (uint64_t *)sram;
        uint64_t *src = (uint64_t *)fwrom;
        int skip_rtl_sim = 1;
        for (int i = 0; i < 8; i++) {
            if (src[i] != dst[i]) {
                skip_rtl_sim = 0;
                break;
            }
        }
        if (skip_rtl_sim) {
            print_uart("NO.COPY. .rtl.sim\r\n", 19);
        } else {
            print_uart("Coping . .FWIMAGE\r\n", 19);
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
    /*gptimers_map *tmr = (gptimers_map *)ADDR_BUS0_XSLV_GPTIMERS;
    uint64_t start = tmr->highcnt;
    while (1) {
        if (tmr->highcnt < start || (start + SYS_HZ) < tmr->highcnt) {
            start = tmr->highcnt;
            print_uart("HIGHCNT: ", 9);
            print_uart_hex(start);
            print_uart("\r\n", 2);
        }
    }*/
}

// Enable U,S,MPRV for all regions
void init_pmp() {
    uint64_t a0 = -1;  // Give S-mode free rein of everything else.
    uint64_t cfg = (PMP_NAPOT | PMP_R | PMP_W | PMP_X) << 0; // [] channel: whole memory range

    // Plug it all in.
    asm volatile ("csrw pmpaddr0, %[a0]\n\t"
                  "csrw pmpcfg0, %[cfg]"
                  :: [a0] "r" (a0),
                     [cfg] "r" (cfg));
}

void _init() {
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    gpio_map *gpio = (gpio_map *)ADDR_BUS0_XSLV_GPIO;
    uart_txctrl_type txctrl;
    uart_rxctrl_type rxctrl;
  
    // mask all interrupts in interrupt controller to avoid
    // unpredictable behaviour after elf-file reloading via debug port.
    uint64_t t1 = 0x00000008;
    asm("csrc mstatus, %0" : :"r"(t1));  // clear mie
    t1 = 0x00000800;
    asm("csrc mie, %0" : :"r"(t1));  // disable external irq from PLIC

    t1 = 0x00000007;
    asm("csrw mcounteren, %0" : :"r"(t1));  // allow counter access from S-mode
    asm("csrw scounteren, %0" : :"r"(t1));  // allow counter access from U-mode

    init_pmp();

    txctrl.v = 0;
    txctrl.b.txen = 1;
    rxctrl.v = 0;
    rxctrl.b.rxen = 1;
    uart->scaler = SYS_HZ / 115200 / 2;  // 40 MHz
    uart->txctrl = txctrl.v;
    uart->rxctrl = rxctrl.v;

    gpio->input_en = 0xF;  // [3:0] input DIP; [11:4] output LEDs
    gpio->output_en = 0xFF0;  

    led_set(0x01);

    copy_image();
    led_set(0x02);

    printf_uart("FPL. . . .%s\r\n", "DONE");
    printf_uart("SPL. . . .%s\r\n", "Started");

    led_set(0x03);

    set_csr(mstatus, MSTATUS_MPP_M);             //      run bbl-q and riscv-tests in machine mode
    if (get_dips() == 0x0F) {
        write_csr(mepc, 0x0000000080000000ull);  // sim: jump to ddr (bbl-q should be init)
    } else {
        write_csr(mepc, 0x0000000008000000ull);  // jump to entry point in SRAM = 0x08000000
    }
}

/** Not used actually */
int main() {
    while (1) {}

    return 0;
}
