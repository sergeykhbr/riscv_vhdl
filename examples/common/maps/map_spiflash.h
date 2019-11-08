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

#ifndef __MAP_SPI_FLASH_H__
#define __MAP_SPI_FLASH_H__

#include <inttypes.h>

typedef union Reg64Type {
    char b8[8];
    uint32_t b32[2];
    uint64_t val;
} Reg64Type;


typedef struct spiflash_map {
    // Can use 4 or 8 bytes access to page buffer
    Reg64Type data[1 << 14];
    // All control register 32-bits width, bits[63:32] - unused
    Reg64Type scaler;             // [RW] no SPI access
    uint32_t rsrv2[2];
    Reg64Type flash_status;       // [RW] with SPI access
    Reg64Type flash_id;           // [RO] with SPI access
    Reg64Type write_ena;          // [WO] Write Enable
    Reg64Type write_page;         // [WO] Write 256 Bytes from the page buffer
    Reg64Type write_dis;          // [WO] Write Enable
    Reg64Type page_erase;         // [WO] Erase Page. Value = address[23:0]
    Reg64Type sector_erase;       // [WO] Erase Page. Value = address[23:0]
    Reg64Type chip_erase;         // [WO] Erase Page. Value any
    Reg64Type pwrdown_ena;        // [WO] Power-Down mode enable. Value any
} spiflash_map;

#endif  // __MAP_SPI_FLASH_H__
