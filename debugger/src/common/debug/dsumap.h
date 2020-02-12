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

#ifndef __DEBUGGER_SRC_COMMON_DEBUG_DSUMAP_H__
#define __DEBUGGER_SRC_COMMON_DEBUG_DSUMAP_H__

#include <inttypes.h>

namespace debugger {

const uint64_t REG_ADDR_ERROR = 0xFFFFFFFFFFFFFFFFull;
const uint64_t DSU_OFFSET  = 0x80080000ull;

union GenericCpuControlType {
    uint64_t val;
    struct {
        uint64_t halt     : 1;
        uint64_t stepping : 1;
        uint64_t sw_breakpoint : 1;
        uint64_t hw_breakpoint : 1;
        uint64_t core_id  : 16;
        uint64_t rsv2     : 12;
        uint64_t istate   : 2;  // [33:32] icache state
        uint64_t rsv3     : 2;  // [35:34] 
        uint64_t dstate   : 2;  // [37:36] dcache state
        uint64_t rsv4     : 2;  // [39:38]
        uint64_t cstate   : 2;  // [41:40] cachetop state
        uint64_t rsv5     : 22;
    } bits;
};

struct DsuMapType {
    // Base Address + 0x00000 (Region 0)
    uint64_t csr[1 << 12];
    // Base Address + 0x08000 (Region 1)
    union ureg_type {
        uint8_t buf[1 << (12 + 3)];
        struct regs_type {
            uint64_t iregs[32];         // integer registers
            uint64_t pc;
            uint64_t npc;
            uint64_t stack_trace_cnt;   // index 34
            uint64_t rsrv1[64 - 35];
            uint64_t fregs[32];         // fpu registers
            uint64_t rsrv2[64 - 32];
            uint64_t stack_trace_buf[1];
            uint64_t rsrv3[128 - 1];
            uint64_t dbg1[8];
        } v;
    } ureg;
    // Base Address + 0x10000 (Region 2)
    union udbg_type {
        uint8_t buf[1 << (12 + 3)];
        struct debug_region_type {
            GenericCpuControlType control;
            uint64_t stepping_mode_steps;
            uint64_t clock_cnt;
            uint64_t executed_cnt;
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
            } br_ctrl;
            uint64_t add_breakpoint;
            uint64_t remove_breakpoint;
            /**
             * Don't fetch instruction from this address use specified
             * below instead.
             */
            uint64_t br_address_fetch;
            /**
             * True instruction value instead of injected one. Use this
             * instruction instead of memory.
             */
            uint64_t br_instr_fetch;
            /**
             * Flush software instruction address from instruction cache.
             */
            uint64_t br_flush_addr;
        } v;
    } udbg;
    // Base Address + 0x18000 (Region 3)
    union local_regs_type {
        uint8_t buf[1 << (12 + 3)];
        struct local_region_type {
            uint64_t soft_reset;
            uint64_t cpu_context;
            uint64_t rsrv[6];
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

#endif  // __DEBUGGER_SRC_COMMON_DEBUG_DSUMAP_H__
