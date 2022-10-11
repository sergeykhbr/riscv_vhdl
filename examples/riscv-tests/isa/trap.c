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

typedef void (*IRQ_HANDLER)(int idx, void *args);

typedef union csr_mcause_type {
    struct bits_type {
        uint64_t code   : 63;   // 11 - Machine external interrupt
        uint64_t irq    : 1;
    } bits;
    uint64_t value;
} csr_mcause_type;

int get_mcause() {
    int ret;
    asm("csrr %0, mcause" : "=r" (ret));
    return ret;
}

int get_mepc() {
    int ret;
    asm("csrr %0, mepc" : "=r" (ret));
    return ret;
}

void print_uart(const char *buf, int sz) {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_txdata_type txdata;
    for (int i = 0; i < sz; i++) {
        do {
            txdata.v = uart->txdata;
        } while (txdata.b.full);
        uart->txdata = buf[i];
    }
}

void print_uart_hex(long val) {
    unsigned char t, s;
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;
    uart_txdata_type txdata;
    for (int i = 0; i < 16; i++) {
        do {
            txdata.v = uart->txdata;
        } while (txdata.b.full);;
        
        t = (unsigned char)((val >> ((15 - i) * 4)) & 0xf);
        if (t < 10) {
            s = t + '0';
        } else {
            s = (t - 10) + 'a';
        }
        uart->txdata = s;
    }
}


void env_call(long long test_id) {
    if (test_id != 0) {
        int mbadaddr;
        print_uart("TEST_FAILED\r\n", 13);
        print_uart("a0=", 3);
        print_uart_hex(test_id);
        print_uart("\r\n", 2);

        asm("csrr %0, mbadaddr" : "=r" (mbadaddr));
        print_uart("mbadaddr=", 9);
        print_uart_hex(mbadaddr);
        print_uart("\r\n", 2);
    } else {
        print_uart("TEST_PASSED\r\n", 13);
    }
    while (1) {}
}

void exception_handler_c(long long arg) {
    int mcause = get_mcause();

    switch (mcause) {
    case 8:  // user env. call
    case 9:  // supervisor env. call
        env_call(arg);
        return;
    default:;
    }

    print_uart("mcause:", 7);
    print_uart_hex(mcause);
    print_uart(",mepc:", 6);
    print_uart_hex(get_mepc());
    print_uart("\r\n", 2);

    /// Exception trap
    while (1) {}
}

