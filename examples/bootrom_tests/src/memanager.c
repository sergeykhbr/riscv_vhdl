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

#include <string.h>
#include <axi_maps.h>
#include "fw_api.h"

void fw_malloc_init() {
    malloc_type *pool = (malloc_type *)ADDR_BUS0_XSLV_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
   // 8-bytes alignment
    pool->allocated_sz = (sizeof(malloc_type) + 7) & ~0x7ull;
    pool->end = (ADDR_BUS0_XSLV_SRAM + pool->allocated_sz);
    pool->data_cnt = 0;
    __sync_synchronize();   // gcc mem barrier to avoi re-ordering

    // RTL Simulation Debug purpose
    pnp->malloc_addr = pool->end;
    pnp->malloc_size = pool->allocated_sz;
}

void *fw_malloc(int size) {
    malloc_type *pool = (malloc_type *)ADDR_BUS0_XSLV_SRAM;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    void *ret = (void *)pool->end;
    pool->allocated_sz += (size + 7) & ~0x7ull;
    pool->end = (ADDR_BUS0_XSLV_SRAM + pool->allocated_sz);

    // RTL Simulation Debug purpose
    pnp->malloc_addr = pool->end;
    pnp->malloc_size = pool->allocated_sz;
    return ret;  
}

void fw_register_ram_data(const char *name, void *data) {
    malloc_type *pool = (malloc_type *)ADDR_BUS0_XSLV_SRAM;
    ram_data_type *p = &pool->data[pool->data_cnt++];
    memcpy(p->name, name, 8);
    p->pattern = data;
}

void *fw_get_ram_data(const char *name) {
    malloc_type *pool = (malloc_type *)ADDR_BUS0_XSLV_SRAM;
    ram_data_type *p;
    for (int i = 0; i < pool->data_cnt; i++) {
        p = &pool->data[i];
        if (strcmp(name, p->name) == 0) {
            return p->pattern;
        }
    }
    return 0;
}
