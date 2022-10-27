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
