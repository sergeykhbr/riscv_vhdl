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
#include <axi_maps.h>

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

uint64_t get_dev_bar(uint16_t vid, uint16_t did) {
    master_cfg_type mcfg;
    slave_cfg_type scfg;
    pnp_map *pnp = (pnp_map *)ADDR_BUS0_XSLV_PNP;
    int slv_total = (pnp->tech >> 8) & 0xFF;
    int mst_total = (pnp->tech >> 16) & 0xFF;
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

    return ~0ull;
}
