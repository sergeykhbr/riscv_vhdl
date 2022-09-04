/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __MAP_PNP_H__
#define __MAP_PNP_H__

#include <inttypes.h>

#define DEV_NONE (~0ull)

typedef struct master_cfg_bits_type {
    uint32_t descrsize : 8;
    uint32_t descrtype : 2;
    uint32_t rsrv : 22;
    uint32_t did : 16;
    uint32_t vid : 16;
} master_cfg_bits_type;

typedef union master_cfg_type {
    master_cfg_bits_type u;
    uint64_t v[1];
} master_cfg_type;


typedef struct slave_cfg_bits_type {
    uint32_t descrsize : 8;
    uint32_t descrtype : 2;
    uint32_t rsrv : 22;
    uint32_t did : 16;
    uint32_t vid : 16;
    uint32_t xmask;
    uint32_t xaddr;
} slave_cfg_bits_type;

typedef union slave_cfg_type {
    slave_cfg_bits_type u;
    uint64_t v[2];
} slave_cfg_type;


typedef struct pnp_map {
    uint32_t hwid;              /// 0xfffff000: RO: HW ID
    uint32_t fwid;              /// 0xfffff004: RW: FW ID
    uint32_t cfg;               /// 0x008: RO target configuration: cpu_max, mst_max, slv_max, plic_irq_max
    uint32_t rsrv1;             /// 0xfffff00c: 
    uint64_t idt;               /// 0xfffff010: 
    uint64_t malloc_addr;       /// 0xfffff018: RW: debuggind memalloc pointer 0x18
    uint64_t malloc_size;       /// 0xfffff020: RW: debugging memalloc size 0x20
    uint64_t fwdbg1;            /// 0xfffff028: RW: FW debug register 1
    uint64_t fwdbg2;            /// 0xfffff030: RW: FW debug register 2
    uint64_t rsrv[1];           /// 0xfffff038
    uint8_t cfg_table[(1 << 12) - 0x40];/// 0xfffff040: RO: PNP configuration
} pnp_map;


static uint64_t get_dev_bar(pnp_map *pnp, uint16_t vid, uint16_t did) {
    master_cfg_type mcfg;
    slave_cfg_type scfg;
    int slv_total = (pnp->cfg >> 8) & 0xFF;
    int mst_total = (pnp->cfg >> 16) & 0xFF;
    int off = 0;

    // skip all masters
    for (int i = 0; i < mst_total; i++) {
        mcfg.v[0] = *(uint64_t *)&pnp->cfg_table[off];
        off += pnp->cfg_table[off];
    }

    for (int i = 0; i < slv_total; i++) {
        scfg.v[0] = *(uint64_t *)&pnp->cfg_table[off];
        scfg.v[1] = *(uint64_t *)&pnp->cfg_table[off + 8];

        if (scfg.u.vid == vid && scfg.u.did == did) {
            return scfg.u.xaddr;
        }
        off += pnp->cfg_table[off];
    }

    return DEV_NONE;
}

#endif  // __MAP_PNP_H__
