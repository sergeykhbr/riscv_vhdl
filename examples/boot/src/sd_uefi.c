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
#include "sd_uefi.h"
#include "spi.h"
#include "uart.h"
#include <string.h>

int is_empty_gui(const struct gpt_guid *guid) {
    return guid->time_low == 0
        && guid->time_mid == 0
        && guid->time_hi_and_version == 0
        && guid->clock_seq_hi == 0
        && guid->clock_seq_low == 0
        && guid->node[0] == 0 && guid->node[1] == 0 && guid->node[2] == 0
        && guid->node[3] == 0 && guid->node[4] == 0 && guid->node[5] == 0;
}



void print_guid(const struct gpt_guid *u)
{
    printf_uart("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        u->time_low >> 24, (u->time_low >> 16) & 0xFF,
        (u->time_low >> 8) & 0xFF, u->time_low & 0xFF,
        u->time_mid >> 8, u->time_mid & 0xFF,
        u->time_hi_and_version >> 8, u->time_hi_and_version & 0xFF,
        u->clock_seq_hi, u->clock_seq_low,
        u->node[0], u->node[1], u->node[2], u->node[3], u->node[4], u->node[5]
        );
}

int is_uboot_guid(const struct gpt_guid *u) {
   struct gpt_guid spl = GPT_LOADER1;
   return spl.time_low == u->time_low
       && spl.time_mid == u->time_mid
       && spl.time_hi_and_version == u->time_hi_and_version
       && spl.clock_seq_hi == u->clock_seq_hi
       && spl.clock_seq_low == u->clock_seq_low
       && spl.node[0] == u->node[0]
       && spl.node[1] == u->node[1]
       && spl.node[2] == u->node[2]
       && spl.node[3] == u->node[3]
       && spl.node[4] == u->node[4]
       && spl.node[5] == u->node[5];
}

/**
 @ret 0 if uefi partion not found, non-zero otherwise
 */
int run_from_sdcard() {
    gpt_legacy_mbr lba0;
    gpt_header lba1;
    gpt_entry entry[4];   // 1 entry = 128 bytes; block = 512 Bytes
    uint64_t lba_start = 0;
    uint64_t lba_end = 0;
    uint8_t *sram = (uint8_t *)ADDR_BUS0_XSLV_SRAM;
    int i4;
   
    init_qspi();

    print_uart("Search uefi\r\n", 13);

    sd_start_reading(0);

    sd_read_block((uint8_t *)&lba0, sizeof(gpt_legacy_mbr));
    sd_read_block((uint8_t *)&lba1, sizeof(gpt_header));

    if (lba1.signature != 0x5452415020494645ULL) {
        sd_stop_reading();
        return -1;
    }

    for (int i = 0; i < lba1.npartition_entries; i++) {
        i4 = i % 4;
        if (i4 == 0) {
            sd_read_block((uint8_t *)entry, sizeof(entry));
        }
        //guid_to_string(&entry[i4].type, guid);

        if (is_empty_gui(&entry[i4].type)) {
            continue;
        }

        printf_uart("[%2d] ", i);
        print_guid(&entry[i4].type);
        print_uart(" : ", 3);
        print_uart((const char *)entry[i4].name, sizeof(entry[i4].name));
        print_uart("\r\n", 2);

        if (is_uboot_guid(&entry[i4].type)) {
            lba_start = entry[i4].lba_start;
            lba_end = entry[i4].lba_end;
        }
    }

    sd_stop_reading();

    if (lba_start == 0) {
        return -1;
    }

    // Coping SPL partition Data
    sd_start_reading(512 * lba_start);

    printf_uart("Coping %d KB", (lba_end - lba_start + 1) / 2);
    for (uint64_t i = lba_start; i <= lba_end; i++) {
        sd_read_block((uint8_t *)entry, sizeof(entry));   // reuse the same buffer
  
        // Check SRAM already initialized with loader1 (sim only)
        if (((uint64_t *)entry)[0] == ((uint64_t *)sram)[0]) {
            printf_uart(". . . . . %s", "SKIPPED(sim)");
            break;
        }

        memcpy(sram, entry, sizeof(entry));
        sram += sizeof(entry);
        if ((i % 100) == 0) {
            print_uart(". ", 2);
        }
    }
    print_uart("\r\n", 2);

    sd_stop_reading();
    return 0;
}
