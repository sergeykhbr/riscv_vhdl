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

#include "axi_maps.h"
#include "spi.h"
#include "uart.h"

#define CMD18 18
#define CMD12 12

void init_qspi() {
    // TODO: initialization for real HW
}

uint8_t sd_get_byte() {
    qspi_map *p = (qspi_map *)ADDR_BUS0_XSLV_QSPI2;
    uint32_t rx = 0;
    do {
        rx = p->rxdata;
    } while (rx & QSPI_RXDATA_EMPTY);
    return (uint8_t)rx;
}

void sd_set_byte(uint8_t v) {
    qspi_map *p = (qspi_map *)ADDR_BUS0_XSLV_QSPI2;
    while (p->txdata & QSPI_TXDATA_FULL) {}
    p->txdata = v;
}


void sd_read_block(uint8_t *buf, int sz) {
    qspi_map *p = (qspi_map *)ADDR_BUS0_XSLV_QSPI2;
    uint8_t token;
    uint8_t crc;

    token = sd_get_byte();
    // TODO check token
    while (sz--) {
       *buf = sd_get_byte();
       buf++;
    }
    crc = sd_get_byte();
    crc = sd_get_byte();
}

int sd_start_reading(uint64_t addr) {
    qspi_map *p = (qspi_map *)ADDR_BUS0_XSLV_QSPI2;
    uint8_t cmdresp;
    sd_set_byte(0x40 | CMD18);
    sd_set_byte(0x00);
    sd_set_byte((uint8_t)((addr >> 16) & 0xFF));
    sd_set_byte((uint8_t)((addr >> 8) & 0xFF));
    sd_set_byte((uint8_t)((addr >> 0) & 0xFF));
    sd_set_byte(0xFF);  // CRC

    cmdresp = sd_get_byte();
    return cmdresp;
}

int sd_stop_reading() {
    qspi_map *p = (qspi_map *)ADDR_BUS0_XSLV_QSPI2;
    uint8_t cmdresp;
    sd_set_byte(0x40 | CMD12);
    sd_set_byte(0);
    sd_set_byte(0);
    sd_set_byte(0);
    sd_set_byte(0);
    sd_set_byte(0xFF);  // CRC
    cmdresp = sd_get_byte();

    // TODO: read status
    return cmdresp;
}


