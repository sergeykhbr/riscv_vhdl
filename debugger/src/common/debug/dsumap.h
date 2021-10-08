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

#pragma once

#include <inttypes.h>

namespace debugger {

const uint64_t REG_ADDR_ERROR = 0xFFFFFFFFFFFFFFFFull;
const uint64_t DSU_OFFSET  = 0x80080000ull;

struct DsuMapType {
    // Base Address + 0x00000 (Region 0)
    uint64_t csr[1 << 12];
    // Base Address + 0x08000 (Region 1)
    union ureg_type {
        uint8_t buf[1 << (12 + 3)];
        struct regs_type {
            uint64_t iregs[32];         // integer registers
            uint64_t fregs[32];         // fpu registers
            uint64_t stack_trace_cnt;   // index 64
            uint64_t rsrv2[64 - 1];
            uint64_t stack_trace_buf[1];
            uint64_t rsrv3[128 - 1];
        } v;
    } ureg;
    // Base Address + 0x10000 (Region 2)
    union udbg_type {
        uint8_t buf[1 << (12 + 3)];
        struct debug_region_type {
            uint64_t reserved_0x00_0x18[4];             // 0x0000..0x0018
            union breakpoint_control_reg {
                uint64_t val;
                struct {
                    /** Trap on instruction:
                     *      0 = Halt pipeline on ECALL instruction
                     *      1 = Generate trap on ECALL instruction
                     */
                    uint64_t trap_on_break : 1;
                    uint64_t rsv1          : 63;
                } bits;
            } br_ctrl;                      // 0x0020
            uint64_t add_breakpoint;        // 0x0028
            uint64_t remove_breakpoint;     // 0x0030
            /**
             * Don't fetch instruction from this address use specified
             * below instead.
             */
            uint64_t rsrv_br_address_fetch;      // 0x0038
            /**
             * True instruction value instead of injected one. Use this
             * instruction instead of memory.
             */
            uint64_t rsrv_br_instr_fetch;        // 0x0040
            /**
             * Flush software instruction address from instruction cache.
             */
            uint64_t rsrv_br_flush_addr;         // 0x0048
        } v;
    } udbg;
    // Base Address + 0x18000 (Region 3)
    union local_regs_type {
        uint8_t buf[1 << (12 + 3)];
        struct local_region_type {
            uint64_t rsrv1[4];
            uint64_t data[12];      // 0x04..0x0f
            uint64_t dmcontrol;     // 0x10
            uint64_t dmstatus;      // 0x11
            uint64_t hartinfo;      // 0x12
            uint64_t haltsum1;      // 0x13
            uint64_t hawindowsel;   // 0x14
            uint64_t hawindow;      // 0x15
            uint64_t abstratcs;     // 0x16
            uint64_t command;       // 0x17
            uint64_t abstratauto;   // 0x18
            uint64_t confstrptr[4]; // 0x19..0x1c
            uint64_t nextdm;        // 0x1d
            uint64_t rsrv[2];       // 0x1e..0x1f
            uint64_t progbuf[16];   // 0x20..0x2f
            uint64_t authdata;      // 0x30
            uint64_t rsrv3[3];      // 0x31..0x33
            uint64_t haltsum2;      // 0x34
            uint64_t haltsum3;      // 0x35
            uint64_t rsrv4;         // 0x36
            uint64_t sbaddress3;    // 0x37
            uint64_t sbcs;          // 0x38
            uint64_t sbaddress[3];  // 0x39..0x3b
            uint64_t sbdata[4];     // 0x3c..0x3f
            uint64_t haltsum0;      // 0x40 hart halted summary
            // Bus utilization registers
            struct mst_bus_util_type {
                uint64_t w_cnt;
                uint64_t r_cnt;
            } bus_util[8];
        } v;
    } ulocal;
};


#define DSUREG(x) (0xFFFFFFFFull & reinterpret_cast<uint64_t>(& \
        (reinterpret_cast<DsuMapType*>(0))->x))
#define DSUREGBASE(x) (DSU_OFFSET + \
        (0xFFFFFFFFull & reinterpret_cast<uint64_t>(& \
        (reinterpret_cast<DsuMapType*>(0))->x)))

struct ECpuRegMapping {
    const char name[16];
    int size;
    uint64_t offset;
};

}  // namespace debugger

