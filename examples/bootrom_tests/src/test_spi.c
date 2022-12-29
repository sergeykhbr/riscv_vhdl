/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

static int32_t spi_is_idle() {
    qspi_map *qspi = (qspi_map *)ADDR_BUS1_APB_QSPI2;

    // Bits[5:4] state machine state. 0 = idle    
    if ((qspi->rsrv4 & 0x30) == 0) {
        return 1;
    }
    return 0;
}

static int32_t spi_send_cmd(uint8_t cmd, uint32_t payload) {
    qspi_map *qspi = (qspi_map *)ADDR_BUS1_APB_QSPI2;
    qspi->txdata = 0x40 | cmd;  // [7] 0=start bit; [6] 1=host request
    qspi->txdata = payload & 0xFF;
    qspi->txdata = (payload >> 8) & 0xFF;
    qspi->txdata = (payload >> 16) & 0xFF;
    qspi->txdata = (payload >> 24) & 0xFF;

    qspi->rsrv4 = (5 << 16) | (1 << 7);  // send cmd + 4 bytes + generate CRC7 byte
    return 0;
}

static int32_t spi_read(uint32_t bytes) {
    qspi_map *qspi = (qspi_map *)ADDR_BUS1_APB_QSPI2;
    qspi->rsrv4 = (bytes << 16);  // bytes to read
    while (!spi_is_idle()) {}
    return 0;
}

int test_spi(void) {
    qspi_map *qspi = (qspi_map *)ADDR_BUS1_APB_QSPI2;
    uint32_t rdata;

    qspi->sckdiv = 8;

    spi_send_cmd(0x00, 0x0);
#if 0
    // CMD0
    qspi->txdata = 0x40;
    qspi->txdata = 0x00;
    qspi->txdata = 0x00;
    qspi->txdata = 0x00;
    qspi->txdata = 0x00; // CRC7 0x95
#endif
#if 0
    // CMD17
    qspi->txdata = 0x51;
    qspi->txdata = 0x00;
    qspi->txdata = 0x00;
    qspi->txdata = 0x00;
    qspi->txdata = 0x00; // CRC7 0x55
#endif
#if 0
    // response CMD17
    qspi->txdata = 0x11;
    qspi->txdata = 0x00;
    qspi->txdata = 0x00;
    qspi->txdata = 0x09;
    qspi->txdata = 0x00; // CRC7 0x67
#endif

    printf_uart("%s", "CMD0: ");
    while (!spi_is_idle()) {}

    rdata = qspi->rxdata;
    while ((rdata & QSPI_RXDATA_EMPTY) == 0) {
        printf_uart("%02x ", rdata & 0xFF);
        rdata = qspi->rxdata;
    }
    printf_uart("%s", "\r\n");

    printf_uart("%s", "R1: ");
    spi_read(1);

    rdata = qspi->rxdata;
    while ((rdata & QSPI_RXDATA_EMPTY) == 0) {
        printf_uart("%02x ", rdata & 0xFF);
        rdata = qspi->rxdata;
    }
    printf_uart("%s", "\r\n");
    return 0;
}

