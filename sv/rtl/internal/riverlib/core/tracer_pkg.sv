// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
package tracer_pkg;

import river_cfg_pkg::*;

localparam int TRACE_TBL_ABITS = 6;
localparam int TRACE_TBL_SZ = 64;
localparam string rname[0:64-1] = '{
    "zero",  // x0
    "ra",  // x1
    "sp",  // x2
    "gp",  // x3
    "tp",  // x4
    "t0",  // x5
    "t1",  // x6
    "t2",  // x7
    "s0",  // x8
    "s1",  // x9
    "a0",  // x10
    "a1",  // x11
    "a2",  // x12
    "a3",  // x13
    "a4",  // x14
    "a5",  // x15
    "a6",  // x16
    "a7",  // x17
    "s2",  // x18
    "s3",  // x19
    "s4",  // x20
    "s5",  // x21
    "s6",  // x22
    "s7",  // x23
    "s8",  // x24
    "s9",  // x25
    "s10",  // x26
    "s11",  // x27
    "t3",  // x28
    "t4",  // x29
    "t5",  // x30
    "t6",  // x31
    "ft0",  // x32
    "ft1",  // x33
    "ft2",  // x34
    "ft3",  // x35
    "ft4",  // x36
    "ft5",  // x37
    "ft6",  // x38
    "ft7",  // x39
    "fs0",  // x40
    "fs1",  // x41
    "fa0",  // x42
    "fa1",  // x43
    "fa2",  // x44
    "fa3",  // x45
    "fa4",  // x46
    "fa5",  // x47
    "fa6",  // x48
    "fa7",  // x49
    "fs2",  // x50
    "fs3",  // x51
    "fs4",  // x52
    "fs5",  // x53
    "fs6",  // x54
    "fs7",  // x55
    "fs8",  // x56
    "fs9",  // x57
    "fs10",  // x58
    "fs11",  // x59
    "ft8",  // x60
    "ft9",  // x61
    "ft10",  // x62
    "ft11"  // x63
};

typedef struct {
    logic store;                                            // 0=load;1=store
    logic [1:0] size;
    logic [63:0] mask;
    logic [63:0] memaddr;
    logic [63:0] data;
    logic [5:0] regaddr;                                    // writeback address
    logic complete;
    logic sc_release;
    logic ignored;
} MemopActionType;

typedef struct {
    logic [5:0] waddr;
    logic [63:0] wres;
} RegActionType;

typedef struct {
    logic [63:0] exec_cnt;
    logic [63:0] pc;
    logic [31:0] instr;
    logic [31:0] regactioncnt;
    logic [31:0] memactioncnt;
    RegActionType regaction[0: TRACE_TBL_SZ - 1];
    MemopActionType memaction[0: TRACE_TBL_SZ - 1];
    logic completed;
} TraceStepType;


typedef struct {
    TraceStepType trace_tbl[0: TRACE_TBL_SZ - 1];
    logic [TRACE_TBL_ABITS-1:0] tr_wcnt;
    logic [TRACE_TBL_ABITS-1:0] tr_rcnt;
    logic [TRACE_TBL_ABITS-1:0] tr_total;
    logic [TRACE_TBL_ABITS-1:0] tr_opened;
} Tracer_registers;

endpackage: tracer_pkg
