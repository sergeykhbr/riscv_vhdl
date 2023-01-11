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

#define CMD0 0
#define CMD8 8

typedef struct SpiDriverDataType {
    qspi_map *map;
    int rxcnt;
    uint8_t rxbuf[256];
} SpiDriverDataType;

static int32_t spi_is_idle(SpiDriverDataType *p) {
    // Bits[5:4] state machine state. 0 = idle    
    if ((p->map->rsrv4 & 0x30) == 0) {
        return 1;
    }
    return 0;
}

static int32_t spi_send_cmd(SpiDriverDataType *p, uint8_t cmd, uint32_t payload) {
    p->map->txdata = 0x40 | cmd;  // [7] 0=start bit; [6] 1=host request
    p->map->txdata = payload & 0xFF;
    p->map->txdata = (payload >> 8) & 0xFF;
    p->map->txdata = (payload >> 16) & 0xFF;
    p->map->txdata = (payload >> 24) & 0xFF;

    p->map->rsrv4 = (5 << 16) | (1 << 7);  // send cmd + 4 bytes + generate CRC7 byte
    while (!spi_is_idle(p)) {}
    return 0;
}

static int32_t spi_send_byte(SpiDriverDataType *p, uint8_t data) {
    p->map->txdata = data;
    p->map->rsrv4 = (1 << 16);  // transmit bytes 1 no CRC
    while (!spi_is_idle(p)) {}
    return 0;
}

static int32_t spi_send_dummy(SpiDriverDataType *p, uint32_t cnt) {
    p->map->rsrv4 = (cnt << 16);  // transmit bytes 1 no CRC
    while (!spi_is_idle(p)) {}
    return 0;
}


int read_rx_fifo(SpiDriverDataType *p) {
    int ret = 0;
    uint32_t rdata;

    rdata = p->map->rxdata;
    while ((rdata & QSPI_RXDATA_EMPTY) == 0) {
        if (p->rxcnt < sizeof(p->rxbuf)) {
            p->rxbuf[p->rxcnt++] = (uint8_t)(rdata & 0xFF);
        }
        printf_uart("%02x ", rdata & 0xFF);
        rdata = p->map->rxdata;
        ret++;
    }
    return ret;
}


int test_spi(void) {
    qspi_map *qspi = (qspi_map *)ADDR_BUS1_APB_QSPI2;
    clint_map *clint = (clint_map *)ADDR_BUS0_XSLV_CLINT;
    int retry_cnt = 0;

    SpiDriverDataType *p = (SpiDriverDataType *)fw_malloc(sizeof(SpiDriverDataType));
    memset(p, 0, sizeof(SpiDriverDataType));
    p->map = (qspi_map *)ADDR_BUS1_APB_QSPI2;

    fw_register_ram_data("spi", p);

    qspi->sckdiv = 8;    // half period

    // Send 10 dummy bytes:
    printf_uart("%s", "DUM: ");
    spi_send_dummy(p, 10);
    read_rx_fifo(p);
    p->rxcnt = 0;
    printf_uart("%s", "\r\n");


    printf_uart("%s", "CMD0: ");
    spi_send_cmd(p, CMD0, 0x0);
    read_rx_fifo(p);
    p->rxcnt = 0;
    printf_uart("%s", "\r\n");
    printf_uart("mosi,wp,cd: %x\r\n", qspi->rsrv4 & 0x7);

    retry_cnt = 0;
    printf_uart("%s", "R1: ");
    while (++retry_cnt < 5) {
        spi_send_byte(p, 0xff);
        read_rx_fifo(p);
        p->rxcnt = 0;
        if (p->rxbuf[0] == 0x01) {
            // Card in idle state
            break;
        }
    }
    printf_uart("%s", "\r\n");
    

    // payload[7:0] check pattern. Recommended 10101010b
    // payload[11:8] voltage supplied (VHS)
    //               0000 Not defined
    //               0001 2.7-3.6V
    //               0010 Reserved for Low Voltage Range
    //               0100 Reserved
    //               1000 Reserved
    //               Others Not defined
    printf_uart("%s", "CMD8: ");
    spi_send_cmd(p, CMD8, 0x1AA);
    read_rx_fifo(p);
    p->rxcnt = 0;
    printf_uart("%s", "\r\n");

    retry_cnt = 0;
    printf_uart("%s", "R1: ");
    while (++retry_cnt < 5) {
        spi_send_byte(p, 0xff);
        read_rx_fifo(p);
        p->rxcnt = 0;
        if (p->rxbuf[0] == 0x01) {
            // Card in idle state
            break;
        }
    }
    printf_uart("%s", "\r\n");

    return 0;
}

