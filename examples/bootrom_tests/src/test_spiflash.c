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

#include <inttypes.h>
#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

//#define TEST_FLASH_WRITE

void flash_wait_ready(spiflash_map *spi) {
    while (spi->flash_status.val & 0x1) {}
}

void test_spiflash(uint64_t bar) {
    spiflash_map *spi = (spiflash_map *)bar;

    // Scaler register is in the cached region. So we have to use MPU to disable caching
    // or use FENCE instruction. Otherwise SPI controller will hang-on system
    mpu_enable_region(3, bar, 256, 0, "rw");
    spi->scaler.b32[0] = 4;  // 10 MHz spi interface

    printf_uart("FlashID. . . . .%02x\r\n", spi->flash_id.b32[0]);

    if (is_simulation() == 0) {
        return;
    }

    spi->data[0].val = 0x8877665544332211ull;

#ifdef TEST_FLASH_WRITE
    spi->page_erase.val = 0x10;
    flash_wait_ready(spi);

    spi->write_ena.val = 1;
    flash_wait_ready(spi);

    spi->write_page.val = 0x10;
    flash_wait_ready(spi);

    spi->write_dis.val = 1;
    flash_wait_ready(spi);
#endif
}