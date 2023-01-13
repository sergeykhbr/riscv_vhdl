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
#define CMD1 1    // Card initialization
#define CMD8 8
#define CMD58 58
#define CMD12 12  // Stop transmission (for Multiple Block Read)
#define CMD17 17  // Read Single Block 
#define CMD18 18  // Multiple Read Operation
#define ACMD41 41
#define CMD55 55  // Next command is an application specific command ACMD
#define DATA_START_BLOCK 0xFE     // for Single Block Read, Single Block Write and Multiple Block Read

typedef enum ESdCardType {
    SD_Unknown,
    SD_Ver1x,
    SD_Ver2x_StandardCapacity,
    SD_Ver2x_HighCapacity,
} ESdCardType;

typedef struct SpiDriverDataType {
    qspi_map *map;
    ESdCardType etype;
    int rxcnt;
    uint8_t rxbuf[512 + 4]; // data start byte + 512 data block + 2 x CRC15 + Dummy
} SpiDriverDataType;

static int32_t spi_is_idle(SpiDriverDataType *p) {
    // Bits[5:4] state machine state. 0 = idle    
    if ((p->map->rsrv4 & 0x30) == 0) {
        return 1;
    }
    return 0;
}

static int32_t spi_send_dummy(SpiDriverDataType *p, uint32_t cnt) {
    p->map->rsrv4 = (cnt << 16);  // transmit bytes 1 no CRC
    while (!spi_is_idle(p)) {}
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

    spi_send_dummy(p, 1);
    return 0;
}

static int32_t spi_send_byte(SpiDriverDataType *p, uint8_t data) {
    p->map->txdata = data;
    p->map->rsrv4 = (1 << 16);  // transmit bytes 1 no CRC
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
        rdata = p->map->rxdata;
        ret++;
    }
    p->rxcnt = 0;  // prepare for the next read
    return ret;
}

uint8_t get_r1_response(SpiDriverDataType *p) {
    uint8_t ret;
    spi_send_byte(p, 0xff);
    read_rx_fifo(p);
    ret = p->rxbuf[0];
    return ret;
}

uint32_t get_r3_ocr(SpiDriverDataType *p) {
    uint32_t ret = 0;
    int rxcnt;
    spi_send_dummy(p, 4);
    rxcnt = read_rx_fifo(p);
    for (int i = 0; i < rxcnt; i++) {
        ret = (ret << 8) | p->rxbuf[i];
    }
    return ret;
}

ESdCardType spi_sd_card_init(SpiDriverDataType *p) {
    int rdcnt;
    uint8_t R1;
    uint32_t R3;
    uint32_t HCS;   // High Capacity Support
    int watchdog;
    p->etype = SD_Unknown;

    // Send 10 dummy bytes:
    printf_uart("%s", "DUM: ");
    spi_send_dummy(p, 10);
    rdcnt = read_rx_fifo(p);
    for (int i = 0; i < rdcnt; i++) {
        printf_uart("%02x ", p->rxbuf[i]);
    }
    printf_uart("%s", "\r\n");

    // Reset SD-card
    printf_uart("%s", "CMD0: ");
    spi_send_cmd(p, CMD0, 0x0);
    rdcnt = read_rx_fifo(p);
    for (int i = 0; i < rdcnt; i++) {
        printf_uart("%02x ", p->rxbuf[i]);
    }

    R1 = get_r1_response(p);
    printf_uart("R1: %02x\r\n", R1);
    

    // Interface Condition Command:
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
    rdcnt = read_rx_fifo(p);
    for (int i = 0; i < rdcnt; i++) {
        printf_uart("%02x ", p->rxbuf[i]);
    }

    R1 = get_r1_response(p);
    printf_uart("R1: %02x\r\n", R1);

    if (R1 == 0x01) {
       // SD-card ver 2 or higher
       p->etype = SD_Ver2x_StandardCapacity;
    } else {
       // SD-card ver 1.x or (Standard Capacity)
       p->etype = SD_Ver1x;
    }

    // CMD58 (Get OCR)
    // Not mandatory to send CMD58. Though it is recommended to be done in order
    // to get the supported voltage range of the card
    // Argument: none; Response R3 (5 bytes R1 + OCR)
    printf_uart("%s", "CMD58: ");
    spi_send_cmd(p, CMD58, 0);
    rdcnt = read_rx_fifo(p);
    for (int i = 0; i < rdcnt; i++) {
        printf_uart("%02x ", p->rxbuf[i]);
    }

    R1 = get_r1_response(p);
    R3 = get_r3_ocr(p);
    printf_uart("R3: %02x %08x\r\n", R1, R3);
    if (R1 != 0x01) {
       p->etype = SD_Unknown;
    }


    // ACMD41. Initialize the card.
    // [31] - reserved
    // [20] - HCS High Capacity Support (OCR)
    // [29:24] - reserved
    // [23:0] -Vdd Voltage Windows
    // Argument: none; Response R1
    watchdog = 0;
    do {
        printf_uart("%s", "CMD55: ");
        spi_send_cmd(p, CMD55, 0);
        rdcnt = read_rx_fifo(p);
        for (int i = 0; i < rdcnt; i++) {
            printf_uart("%02x ", p->rxbuf[i]);
        }
        R1 = get_r1_response(p);
        printf_uart("R1: %02x\r\n", R1);

        printf_uart("%s", "ACMD41: ");
        spi_send_cmd(p, ACMD41, (1 << 30));  // Support High Capacity cards
        rdcnt = read_rx_fifo(p);
        for (int i = 0; i < rdcnt; i++) {
            printf_uart("%02x ", p->rxbuf[i]);
        }
        R1 = get_r1_response(p);
        printf_uart("R1: %02x\r\n", R1);
        if (R1 == 0x05) {
           break;
        }

        // The 'in idle state' bit in the R1 response of ACMD41 is used by the card 
        // to inform the host if initialization of ACMD41 is completed.
        //     Setting this bit to 1 indicates that the card is still initializing.
        //     Setting this bit to 0 indicates completion of initialization.
        // The host repeatedly issues ACMD41 until this bit is set to 0
    } while (((R1 & 0x1) != 0) && watchdog++ < 2);


    if (R1 == 0x05) {
        // ACMD41 is unsupported, use CMD1
        watchdog = 0;
        do {
            printf_uart("%s", "CMD1: ");
            spi_send_cmd(p, CMD1, (1 << 30));  // Support High Capacity cards
            rdcnt = read_rx_fifo(p);
             for (int i = 0; i < rdcnt; i++) {
                printf_uart("%02x ", p->rxbuf[i]);
            }
            R1 = get_r1_response(p);
            printf_uart("R1: %02x\r\n", R1);

        } while (((R1 & 0x1) != 0) && watchdog++ < 2);
    }

    // @warning: Card should return 'in_idle_state = 0', if watchdog then Not SD card
    if (p->etype == SD_Unknown || p->etype == SD_Ver1x) {
        return 0;
    }


    // CMD58 (get CCS). Card capacity information (Standard or Extended)
    // Not mandatory to send CMD58. Though it is recommended to be done in order
    // to get the supported voltage range of the card
    // Argument: none; Response R3 (5 bytes R1 + OCR)
    printf_uart("%s", "CMD58: ");
    spi_send_cmd(p, CMD58, 0);
    rdcnt = read_rx_fifo(p);
    for (int i = 0; i < rdcnt; i++) {
        printf_uart("%02x ", p->rxbuf[i]);
    }
    printf_uart("%s", "\r\n");

    R1 = get_r1_response(p);
    R3 = get_r3_ocr(p);
    HCS = (R3 >> 30) & 0x1;
    printf_uart("R3: %02x %04x HCS=%d\r\n", R1, R3, HCS);

    if (R1 != 0x01) {
       p->etype = SD_Unknown;
    } else if (HCS) {
       p->etype = SD_Ver2x_HighCapacity;
    } else {
       p->etype = SD_Ver2x_StandardCapacity;
    }

    return 0;
}

int spi_sd_card_read(SpiDriverDataType *p, uint64_t addr, int sz) {
    uint32_t sd_addr;
    int block_size = 512;
    int rdcnt;
    uint8_t R1;

    if (p->etype == SD_Ver2x_HighCapacity) {
        sd_addr = (uint32_t)(addr >> 9);  // Data block is always 512 bytes
    } else {
        sd_addr = (uint32_t)addr;
        // Block size could be changed from 512 if partial block is enabled
    }

    printf_uart("%s", "CMD17: ");
    spi_send_cmd(p, CMD17, sd_addr);
    rdcnt = read_rx_fifo(p);
    for (int i = 0; i < rdcnt; i++) {
        printf_uart("%02x ", p->rxbuf[i]);
    }
    R1 = get_r1_response(p);
    printf_uart("R1: %02x\r\n", R1);

    if (R1 != 0x01) {
        return 0;
    }

    // Check Data token:
    int watchdog = 0;
    uint8_t data_prefix = 0;
    printf_uart("%s ", "StartToken: ");

    do {
        spi_send_dummy(p, 1);
        read_rx_fifo(p);
        data_prefix = p->rxbuf[0];
        printf_uart("%02x ", data_prefix);
    } while (data_prefix != DATA_START_BLOCK && watchdog++ < 5);
    printf_uart("%s", "\r\n");

    if (data_prefix == DATA_START_BLOCK) {
        printf_uart("%s ", "DATA: ");
        spi_send_dummy(p, 512);
        read_rx_fifo(p);

        for (int i = 0; i < 8; i++) {
            printf_uart("%02x ", p->rxbuf[i]);
        }

        spi_send_dummy(p, 2); // CRC15
        read_rx_fifo(p);
        printf_uart(".. %02x%02x\r\n", p->rxbuf[0], p->rxbuf[1]);
    } else {
        sz = 0;
    }

    return sz;
}

int test_spi(void) {
    int watchdog;

    SpiDriverDataType *p = (SpiDriverDataType *)fw_malloc(sizeof(SpiDriverDataType));
    memset(p, 0, sizeof(SpiDriverDataType));
    p->map = (qspi_map *)ADDR_BUS1_APB_QSPI2;
    p->map->sckdiv = 8;    // half period

    fw_register_ram_data("spi", p);

    spi_sd_card_init(p);

    spi_sd_card_read(p, 1024, 512);
    return 0;
}

