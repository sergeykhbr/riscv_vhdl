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

#pragma once

#include <inttypes.h>

typedef enum ESdCardType {
    SD_Unknown,
    SD_Ver1x,
    SD_Ver2x_StandardCapacity,
    SD_Ver2x_HighCapacity,
} ESdCardType;


#define QSPI_RXDATA_EMPTY (0x1 << 31)
#define QSPI_TXDATA_FULL  (0x1 << 31)

typedef struct qspi_map {
    volatile uint32_t sckdiv;            // [0x00] Serial clock divisor
    volatile uint32_t sckmode;           // [0x04] Serial clock mode
    volatile uint32_t rsrv1[2];          // [0x08,0x0C]
    volatile uint32_t csid;              // [0x10] Chip select ID
    volatile uint32_t csdef;             // [0x14] Chip select default
    volatile uint32_t csmode;            // [0x18] Chip select mode
    volatile uint32_t rsrv2[3];          // [0x1C,0x20,0x24]
    volatile uint32_t delay0;            // [0x28] Delay control 0
    volatile uint32_t delay1;            // [0x2C] Delay control 1
    volatile uint32_t rsrv3[4];          // [0x30,0x34,0x38,0x3C]
    volatile uint32_t fmt;               // [0x40] Frame format
    volatile uint32_t rsrv4;             // [0x44]
    volatile uint32_t txdata;            // [0x48] Tx FIFO data
    volatile uint32_t rxdata;            // [0x4C] Rx FIFO data
    volatile uint32_t txmark;            // [0x50] Tx FIFO watermark
    volatile uint32_t rxmark;            // [0x54] Rx FIFO watermark
    volatile uint32_t crc16;             // [0x58] CRC16 value (reserved FU740)
    volatile uint32_t rsrv5;             // [0x5C]
    volatile uint32_t fctrl;             // [0x60] SPI flash interface control (available in direct-map only)
    volatile uint32_t ffmt;              // [0x64] SPI flash instruction format (available in direct-map only)
    volatile uint32_t rsrv6[2];          // [0x68,0x6C]
    volatile uint32_t ie;                // [0x70] SPI interrupt enable
    volatile uint32_t ip;                // [0x74] SPI interrupt pending
} qspi_map;

