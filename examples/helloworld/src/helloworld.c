/*****************************************************************************
 * @file
 * @author   Sergey Khabarov
 * @brief    Firmware example. 
 ****************************************************************************/

#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include "axi_maps.h"

extern char _end;

/**
 * @name sbrk
 * @brief Increase program data space.
 * @details Malloc and related functions depend on this.
 */
char *sbrk(int incr) {
    return &_end;
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

void helloWorld() {
    char ss[256];
    int ss_len;
    ss_len = sprintf(ss, "Hellow World - %d!!!!\n", 1);
    print_uart(ss, ss_len);
}

