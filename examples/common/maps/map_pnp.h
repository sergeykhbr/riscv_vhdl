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


typedef struct MasterConfigType {
    /*union DescrType {
        struct bits_type {
            uint32_t descrsize : 8;
            uint32_t descrtype : 2;
            uint32_t rsrv : 14;
            uint32_t xindex : 8;
        } bits;
        uint32_t val;
    } descr;*/
    uint32_t descr;
    uint16_t did;
    uint16_t vid;
} MasterConfigType;

typedef struct SlaveConfigType {
    /*union DescrType {
        struct bits_type {
            uint32_t descrsize : 8;
            uint32_t descrtype : 2;
            uint32_t bar_total : 2;
            uint32_t rsrv1 : 4;
            uint32_t irq_idx : 8;
            uint32_t xindex : 8;
        } bits;
        uint32_t val;
    } descr;*/
    uint32_t descr;
    uint16_t did;
    uint16_t vid;
    uint32_t xmask;
    uint32_t xaddr;
} SlaveConfigType;

typedef struct pnp_map {
    uint32_t hwid;              /// 0xfffff000: RO: HW ID
    uint32_t fwid;              /// 0xfffff004: RW: FW ID
    /*union TechType {
        struct bits_type {
            uint8_t tech;
            uint8_t slv_total;
            uint8_t mst_total;
            uint8_t adc_detect;
        } bits;
        uint32_t val;
    } tech;                     /// 0xfffff008: RO: technology index
    */
    uint32_t tech;
    uint32_t rsrv1;             /// 0xfffff00c: 
    uint64_t idt;               /// 0xfffff010: 
    uint64_t malloc_addr;       /// 0xfffff018: RW: debuggind memalloc pointer 0x18
    uint64_t malloc_size;       /// 0xfffff020: RW: debugging memalloc size 0x20
    uint64_t fwdbg1;            /// 0xfffff028: RW: FW debug register 1
    uint64_t fwdbg2;            /// 0xfffff030: RW: FW debug register 2
    uint64_t rsrv[1];           /// 0xfffff038
    uint8_t cfg_table[(1 << 12) - 0x40];/// 0xfffff040: RO: PNP configuration
} pnp_map;


#endif  // __MAP_PNP_H__
