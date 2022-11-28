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

`timescale 1ns/10ps

module Tracer #(
    parameter bit async_reset = 1'b0,
    parameter int unsigned hartid = 0,
    parameter string trace_file = "trace_river_sysc"
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [63:0] i_dbg_executed_cnt,
    input logic i_e_valid,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_pc,
    input logic [31:0] i_e_instr,
    input logic i_e_wena,
    input logic [5:0] i_e_waddr,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_wdata,
    input logic i_e_memop_valid,
    input logic [river_cfg_pkg::MemopType_Total-1:0] i_e_memop_type,
    input logic [1:0] i_e_memop_size,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_memop_addr,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_memop_wdata,
    input logic i_e_flushd,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_m_pc,     // executed memory/flush request only
    input logic i_m_valid,                                  // memory/flush operation completed
    input logic i_m_memop_ready,
    input logic i_m_wena,
    input logic [5:0] i_m_waddr,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_m_wdata,
    input logic i_reg_ignored
);

import river_cfg_pkg::*;
import tracer_pkg::*;

localparam string rname[0: 64-1] = '{
    "zero",
    "ra",
    "sp",
    "gp",
    "tp",
    "t0",
    "t1",
    "t2",
    "s0",
    "s1",
    "a0",
    "a1",
    "a2",
    "a3",
    "a4",
    "a5",
    "a6",
    "a7",
    "s2",
    "s3",
    "s4",
    "s5",
    "s6",
    "s7",
    "s8",
    "s9",
    "s10",
    "s11",
    "t3",
    "t4",
    "t5",
    "t6",
    "ft0",
    "ft1",
    "ft2",
    "ft3",
    "ft4",
    "ft5",
    "ft6",
    "ft7",
    "fs0",
    "fs1",
    "fa0",
    "fa1",
    "fa2",
    "fa3",
    "fa4",
    "fa5",
    "fa6",
    "fa7",
    "fs2",
    "fs3",
    "fs4",
    "fs5",
    "fs6",
    "fs7",
    "fs8",
    "fs9",
    "fs10",
    "fs11",
    "ft8",
    "ft9",
    "ft10",
    "ft11"
};

string trfilename;                                          // formatted string name with hartid
string outstr;
string tracestr;
int fl;
Tracer_registers r, rin;

function string TaskDisassembler(input logic [31:0] instr);
string ostr;
begin
    if (instr[1: 0] != 2'h3) begin
        case (instr[1: 0])
        2'h0: begin
            case (instr[15: 13])
            3'h0: begin
                if ((|instr[12: 2]) == 1'b0) begin
                    ostr = $sformatf("%10s", "ERROR");
                end else begin
                    ostr = $sformatf("%10s", "c.addi4spn");
                end
            end
            3'h1: begin
                ostr = $sformatf("%10s", "c.fld");
            end
            3'h2: begin
                ostr = $sformatf("%10s", "c.lw");
            end
            3'h3: begin
                ostr = $sformatf("%10s", "c.ld");
            end
            3'h4: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            3'h5: begin
                ostr = $sformatf("%10s", "c.fsd");
            end
            3'h6: begin
                ostr = $sformatf("%10s", "c.sw");
            end
            3'h7: begin
                ostr = $sformatf("%10s", "c.sd");
            end
            endcase
        end
        2'h1: begin
            case (instr[15: 13])
            3'h0: begin
                if ((|instr[12: 2]) == 1'b0) begin
                    ostr = $sformatf("%10s", "c.nop");
                end else begin
                    ostr = $sformatf("%10s", "c.addi");
                end
            end
            3'h1: begin
                if ((|instr[11: 7]) == 1'b0) begin
                    ostr = $sformatf("%10s", "ERROR");
                end else begin
                    ostr = $sformatf("%10s", "c.addiw");
                end
            end
            3'h2: begin
                if ((|instr[11: 7]) == 1'b0) begin
                    ostr = $sformatf("%10s", "ERROR");
                end else begin
                    ostr = $sformatf("%10s", "c.li");
                end
            end
            3'h3: begin
                if (instr[11: 7] == 5'h02) begin
                    ostr = $sformatf("%10s", "c.addi16sp");
                end else if ((|instr[11: 7]) == 1'b1) begin
                    ostr = $sformatf("%10s", "c.lui");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            3'h4: begin
                if (instr[11: 10] == 2'h0) begin
                    if ((instr[12] == 1'b0) && (instr[6: 2] == 5'h00)) begin
                        ostr = $sformatf("%10s", "c.srli64");
                    end else begin
                        ostr = $sformatf("%10s", "c.srli");
                    end
                end else if (instr[11: 10] == 2'h1) begin
                    if ((instr[12] == 1'b0) && (instr[6: 2] == 5'h00)) begin
                        ostr = $sformatf("%10s", "c.srai64");
                    end else begin
                        ostr = $sformatf("%10s", "c.srai");
                    end
                end else if (instr[11: 10] == 2'h2) begin
                    ostr = $sformatf("%10s", "c.andi");
                end else begin
                    if ((instr[12] == 1'b0) && (instr[6: 5] == 2'h0)) begin
                        ostr = $sformatf("%10s", "c.sub");
                    end else if ((instr[12] == 1'b0) && (instr[6: 5] == 2'h1)) begin
                        ostr = $sformatf("%10s", "c.xor");
                    end else if ((instr[12] == 1'b0) && (instr[6: 5] == 2'h2)) begin
                        ostr = $sformatf("%10s", "c.or");
                    end else if ((instr[12] == 1'b0) && (instr[6: 5] == 2'h3)) begin
                        ostr = $sformatf("%10s", "c.and");
                    end else if ((instr[12] == 1'b1) && (instr[6: 5] == 2'h0)) begin
                        ostr = $sformatf("%10s", "c.subw");
                    end else if ((instr[12] == 1'b1) && (instr[6: 5] == 2'h1)) begin
                        ostr = $sformatf("%10s", "c.addw");
                    end else begin
                        ostr = $sformatf("%10s", "ERROR");
                    end
                end
            end
            3'h5: begin
                ostr = $sformatf("%10s", "c.j");
            end
            3'h6: begin
                ostr = $sformatf("%10s", "c.beqz");
            end
            3'h7: begin
                ostr = $sformatf("%10s", "c.bnez");
            end
            endcase
        end
        2'h2: begin
            case (instr[15: 13])
            3'h0: begin
                if ((instr[12] == 1'b0) && (instr[6: 5] == 2'h0)) begin
                    ostr = $sformatf("%10s", "c.slli64");
                end else begin
                    ostr = $sformatf("%10s", "c.slli");
                end
            end
            3'h1: begin
                ostr = $sformatf("%10s", "c.fldsp");
            end
            3'h2: begin
                ostr = $sformatf("%10s", "c.lwsp");
            end
            3'h3: begin
                ostr = $sformatf("%10s", "c.ldsp");
            end
            3'h4: begin
                if ((instr[12] == 1'b0) && (instr[6: 2] == 5'h00)) begin
                    ostr = $sformatf("%10s", "c.jr");
                end else if ((instr[12] == 1'b0) && (instr[6: 2] != 5'h00)) begin
                    ostr = $sformatf("%10s", "c.mv");
                end else if ((instr[12] == 1'b1) && (instr[6: 2] == 5'h00) && (instr[11: 7] == 5'h00)) begin
                    ostr = $sformatf("%10s", "c.ebreak");
                end else if ((instr[12] == 1'b1) && (instr[6: 2] == 5'h00)) begin
                    ostr = $sformatf("%10s", "c.jalr");
                end else begin
                    ostr = $sformatf("%10s", "c.add");
                end
            end
            3'h5: begin
                ostr = $sformatf("%10s", "c.fsdsp");
            end
            3'h6: begin
                ostr = $sformatf("%10s", "c.swsp");
            end
            3'h7: begin
                ostr = $sformatf("%10s", "c.sdsp");
            end
            endcase
        end
        default: begin
            ostr = $sformatf("%10s", "ERROR");
        end
        endcase
    end else begin
        // RV decoder
        case (instr[6: 0])
        7'h03: begin
            case (instr[14: 12])
            3'h0: begin
                ostr = $sformatf("%10s", "lb");
            end
            3'h1: begin
                ostr = $sformatf("%10s", "lh");
            end
            3'h2: begin
                ostr = $sformatf("%10s", "lw");
            end
            3'h3: begin
                ostr = $sformatf("%10s", "ld");
            end
            3'h4: begin
                ostr = $sformatf("%10s", "lbu");
            end
            3'h5: begin
                ostr = $sformatf("%10s", "lhu");
            end
            3'h6: begin
                ostr = $sformatf("%10s", "lwu");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h07: begin
            case (instr[14: 12])
            3'h3: begin
                ostr = $sformatf("%10s", "fld");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h0f: begin
            case (instr[14: 12])
            3'h0: begin
                ostr = $sformatf("%10s", "fence");
            end
            3'h1: begin
                ostr = $sformatf("%10s", "fence.i");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h13: begin
            case (instr[14: 12])
            3'h0: begin
                ostr = $sformatf("%10s", "addi");
            end
            3'h1: begin
                if ((|instr[31: 26]) == 1'b0) begin
                    ostr = $sformatf("%10s", "slli");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            3'h2: begin
                ostr = $sformatf("%10s", "slti");
            end
            3'h3: begin
                ostr = $sformatf("%10s", "sltiu");
            end
            3'h4: begin
                ostr = $sformatf("%10s", "xori");
            end
            3'h5: begin
                if ((|instr[31: 26]) == 1'b0) begin
                    ostr = $sformatf("%10s", "srli");
                end else if (instr[31: 26] == 6'h10) begin
                    ostr = $sformatf("%10s", "srai");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            3'h6: begin
                ostr = $sformatf("%10s", "ori");
            end
            3'h7: begin
                ostr = $sformatf("%10s", "andi");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h17: begin
            ostr = $sformatf("%10s", "auipc");
        end
        7'h1b: begin
            case (instr[14: 12])
            3'h0: begin
                ostr = $sformatf("%10s", "addiw");
            end
            3'h1: begin
                if (instr[31: 25] == 7'h00) begin
                    ostr = $sformatf("%10s", "slliw");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            3'h5: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "srliw");
                end
                7'h20: begin
                    ostr = $sformatf("%10s", "sraiw");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h23: begin
            case (instr[14: 12])
            3'h0: begin
                ostr = $sformatf("%10s", "sb");
            end
            3'h1: begin
                ostr = $sformatf("%10s", "sh");
            end
            3'h2: begin
                ostr = $sformatf("%10s", "sw");
            end
            3'h3: begin
                ostr = $sformatf("%10s", "sd");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h27: begin
            case (instr[14: 12])
            3'h3: begin
                ostr = $sformatf("%10s", "fsd");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h2f: begin
            if (instr[14: 12] == 3'h2) begin
                case (instr[31: 27])
                5'h00: begin
                    ostr = $sformatf("%10s", "amoadd.w");
                end
                5'h01: begin
                    ostr = $sformatf("%10s", "amoswap.w");
                end
                5'h02: begin
                    if ((|instr[24: 20]) == 1'b0) begin
                        ostr = $sformatf("%10s", "lr.w");
                    end else begin
                        ostr = $sformatf("%10s", "ERROR");
                    end
                end
                5'h03: begin
                    ostr = $sformatf("%10s", "sc.w");
                end
                5'h04: begin
                    ostr = $sformatf("%10s", "amoxor.w");
                end
                5'h08: begin
                    ostr = $sformatf("%10s", "amoor.w");
                end
                5'h0c: begin
                    ostr = $sformatf("%10s", "amoand.w");
                end
                5'h10: begin
                    ostr = $sformatf("%10s", "amomin.w");
                end
                5'h14: begin
                    ostr = $sformatf("%10s", "amomax.w");
                end
                5'h18: begin
                    ostr = $sformatf("%10s", "amominu.w");
                end
                5'h1c: begin
                    ostr = $sformatf("%10s", "amomaxu.w");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end else if (instr[14: 12] == 3'h3) begin
                case (instr[31: 27])
                5'h00: begin
                    ostr = $sformatf("%10s", "amoadd.d");
                end
                5'h01: begin
                    ostr = $sformatf("%10s", "amoswap.d");
                end
                5'h02: begin
                    if ((|instr[24: 20]) == 1'b0) begin
                        ostr = $sformatf("%10s", "lr.d");
                    end else begin
                        ostr = $sformatf("%10s", "ERROR");
                    end
                end
                5'h03: begin
                    ostr = $sformatf("%10s", "sc.d");
                end
                5'h04: begin
                    ostr = $sformatf("%10s", "amoxor.d");
                end
                5'h08: begin
                    ostr = $sformatf("%10s", "amoor.d");
                end
                5'h0c: begin
                    ostr = $sformatf("%10s", "amoand.d");
                end
                5'h10: begin
                    ostr = $sformatf("%10s", "amomin.d");
                end
                5'h14: begin
                    ostr = $sformatf("%10s", "amomax.d");
                end
                5'h18: begin
                    ostr = $sformatf("%10s", "amominu.d");
                end
                5'h1c: begin
                    ostr = $sformatf("%10s", "amomaxu.d");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end else begin
                ostr = $sformatf("%10s", "ERROR");
            end
        end
        7'h33: begin
            case (instr[14: 12])
            3'h0: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "add");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "mul");
                end
                7'h20: begin
                    ostr = $sformatf("%10s", "sub");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h1: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "sll");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "mulh");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h2: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "slt");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "mulhsu");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h3: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "sltu");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "mulhu");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h4: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "xor");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "div");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h5: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "srl");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "divu");
                end
                7'h20: begin
                    ostr = $sformatf("%10s", "sra");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h6: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "or");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "rem");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h7: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "and");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "remu");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h37: begin
            ostr = $sformatf("%10s", "lui");
        end
        7'h3b: begin
            case (instr[14: 12])
            3'h0: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "addw");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "mulw");
                end
                7'h20: begin
                    ostr = $sformatf("%10s", "subw");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h1: begin
                if ((|instr[31: 25]) == 1'b0) begin
                    ostr = $sformatf("%10s", "sllw");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            3'h4: begin
                case (instr[31: 25])
                7'h01: begin
                    ostr = $sformatf("%10s", "divw");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h5: begin
                case (instr[31: 25])
                7'h00: begin
                    ostr = $sformatf("%10s", "srlw");
                end
                7'h01: begin
                    ostr = $sformatf("%10s", "divuw");
                end
                7'h20: begin
                    ostr = $sformatf("%10s", "sraw");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h6: begin
                case (instr[31: 25])
                7'h01: begin
                    ostr = $sformatf("%10s", "remw");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            3'h7: begin
                case (instr[31: 25])
                7'h01: begin
                    ostr = $sformatf("%10s", "remuw");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h53: begin
            case (instr[31: 25])
            7'h01: begin
                ostr = $sformatf("%10s", "fadd");
            end
            7'h05: begin
                ostr = $sformatf("%10s", "fsub");
            end
            7'h09: begin
                ostr = $sformatf("%10s", "fmul");
            end
            7'h0d: begin
                ostr = $sformatf("%10s", "fdiv");
            end
            7'h15: begin
                case (instr[14: 12])
                3'h0: begin
                    ostr = $sformatf("%10s", "fmin");
                end
                3'h1: begin
                    ostr = $sformatf("%10s", "fmax");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            7'h51: begin
                case (instr[14: 12])
                3'h0: begin
                    ostr = $sformatf("%10s", "fle");
                end
                3'h1: begin
                    ostr = $sformatf("%10s", "flt");
                end
                3'h2: begin
                    ostr = $sformatf("%10s", "feq");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            7'h61: begin
                case (instr[24: 20])
                5'h00: begin
                    ostr = $sformatf("%10s", "fcvt.w.d");
                end
                5'h01: begin
                    ostr = $sformatf("%10s", "fcvt.wu.d");
                end
                5'h02: begin
                    ostr = $sformatf("%10s", "fcvt.l.d");
                end
                5'h03: begin
                    ostr = $sformatf("%10s", "fcvt.lu.d");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            7'h69: begin
                case (instr[24: 20])
                5'h00: begin
                    ostr = $sformatf("%10s", "fcvt.d.w");
                end
                5'h01: begin
                    ostr = $sformatf("%10s", "fcvt.d.wu");
                end
                5'h02: begin
                    ostr = $sformatf("%10s", "fcvt.d.l");
                end
                5'h03: begin
                    ostr = $sformatf("%10s", "fcvt.d.lu");
                end
                default: begin
                    ostr = $sformatf("%10s", "ERROR");
                end
                endcase
            end
            7'h71: begin
                if (((|instr[24: 20]) == 1'b0) && ((|instr[14: 12]) == 1'b0)) begin
                    ostr = $sformatf("%10s", "fmov.x.d");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            7'h79: begin
                if (((|instr[24: 20]) == 1'b0) && ((|instr[14: 12]) == 1'b0)) begin
                    ostr = $sformatf("%10s", "fmov.d.x");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h63: begin
            case (instr[14: 12])
            3'h0: begin
                ostr = $sformatf("%10s", "beq");
            end
            3'h1: begin
                ostr = $sformatf("%10s", "bne");
            end
            3'h4: begin
                ostr = $sformatf("%10s", "blt");
            end
            3'h5: begin
                ostr = $sformatf("%10s", "bge");
            end
            3'h6: begin
                ostr = $sformatf("%10s", "bltu");
            end
            3'h7: begin
                ostr = $sformatf("%10s", "bgeu");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        7'h67: begin
            ostr = $sformatf("%10s", "jalr");
        end
        7'h6f: begin
            ostr = $sformatf("%10s", "jal");
        end
        7'h73: begin
            case (instr[14: 12])
            3'h0: begin
                if (instr == 32'h00000073) begin
                    ostr = $sformatf("%10s", "ecall");
                end else if (instr == 32'h00100073) begin
                    ostr = $sformatf("%10s", "ebreak");
                end else if (instr == 32'h00200073) begin
                    ostr = $sformatf("%10s", "uret");
                end else if (instr == 32'h10200073) begin
                    ostr = $sformatf("%10s", "sret");
                end else if (instr == 32'h10500073) begin
                    ostr = $sformatf("%10s", "wfi");
                end else if (instr == 32'h20200073) begin
                    ostr = $sformatf("%10s", "hret");
                end else if (instr == 32'h30200073) begin
                    ostr = $sformatf("%10s", "mret");
                end else begin
                    ostr = $sformatf("%10s", "ERROR");
                end
            end
            3'h1: begin
                ostr = $sformatf("%10s", "csrrw");
            end
            3'h2: begin
                ostr = $sformatf("%10s", "csrrs");
            end
            3'h3: begin
                ostr = $sformatf("%10s", "csrrc");
            end
            3'h5: begin
                ostr = $sformatf("%10s", "csrrwi");
            end
            3'h6: begin
                ostr = $sformatf("%10s", "csrrsi");
            end
            3'h7: begin
                ostr = $sformatf("%10s", "csrrci");
            end
            default: begin
                ostr = $sformatf("%10s", "ERROR");
            end
            endcase
        end
        default: begin
            ostr = $sformatf("%10s", "ERROR");
        end
        endcase
    end
    return ostr;
end
endfunction: TaskDisassembler

function string TraceOutput(input logic [TRACE_TBL_ABITS-1:0] rcnt);
string ostr;
begin
    string disasm;
    int ircnt;
    int iwaddr;

    ircnt = int'(rcnt);

    disasm = TaskDisassembler(r.trace_tbl[ircnt].instr);
    ostr += $sformatf("%9d: %08x: ",
            r.trace_tbl[ircnt].exec_cnt,
            r.trace_tbl[ircnt].pc);

    ostr += disasm;
    ostr += " \n";

    for (int i = 0; i < int'(r.trace_tbl[ircnt].memactioncnt); i++) begin
        if (r.trace_tbl[ircnt].memaction[i].ignored == 1'b0) begin
            if (r.trace_tbl[ircnt].memaction[i].store == 1'b0) begin
                ostr += $sformatf("%20s [%08x] => %016x\n",
                        "",
                        r.trace_tbl[ircnt].memaction[i].memaddr,
                        r.trace_tbl[ircnt].memaction[i].data);
            end else begin
                ostr += $sformatf("%20s [%08x] <= %016x\n",
                        "",
                        r.trace_tbl[ircnt].memaction[i].memaddr,
                        r.trace_tbl[ircnt].memaction[i].data);
            end
        end
    end

    for (int i = 0; i < int'(r.trace_tbl[ircnt].regactioncnt); i++) begin
        iwaddr = int'(r.trace_tbl[ircnt].regaction[i].waddr);
        ostr += $sformatf("%20s %10s <= %016x\n",
                "",
                rname[iwaddr],
                r.trace_tbl[ircnt].regaction[i].wres);
    end
    return ostr;
end
endfunction: TraceOutput

initial begin
    trfilename = $sformatf("%s%d.log",
            trace_file,
            hartid);
    fl = $fopen(trfilename, "w");
    assert (fl)
    else begin
        $warning("Cannot open log-file");
    end
end

always_comb
begin: comb_proc
    Tracer_registers v;
    int wcnt;
    int xcnt;
    int rcnt;
    int regcnt;
    int memcnt;
    logic [6:0] mskoff;
    logic [63:0] mask;
    logic [TRACE_TBL_ABITS-1:0] tr_wcnt_nxt;
    logic checked;
    logic entry_valid;
    logic [TRACE_TBL_ABITS-1:0] rcnt_inc;

    wcnt = 0;
    xcnt = 0;
    rcnt = 0;
    regcnt = 0;
    memcnt = 0;
    mskoff = 0;
    mask = 0;
    tr_wcnt_nxt = 0;
    checked = 0;
    entry_valid = 0;
    rcnt_inc = 0;

    for (int i = 0; i < TRACE_TBL_SZ; i++) begin
        v.trace_tbl[i].exec_cnt = r.trace_tbl[i].exec_cnt;
        v.trace_tbl[i].pc = r.trace_tbl[i].pc;
        v.trace_tbl[i].instr = r.trace_tbl[i].instr;
        v.trace_tbl[i].regactioncnt = r.trace_tbl[i].regactioncnt;
        v.trace_tbl[i].memactioncnt = r.trace_tbl[i].memactioncnt;
        for (int j = 0; j < TRACE_TBL_SZ; j++) begin
            v.trace_tbl[i].regaction[j].waddr = r.trace_tbl[i].regaction[j].waddr;
            v.trace_tbl[i].regaction[j].wres = r.trace_tbl[i].regaction[j].wres;
        end
        for (int j = 0; j < TRACE_TBL_SZ; j++) begin
            v.trace_tbl[i].memaction[j].store = r.trace_tbl[i].memaction[j].store;
            v.trace_tbl[i].memaction[j].size = r.trace_tbl[i].memaction[j].size;
            v.trace_tbl[i].memaction[j].mask = r.trace_tbl[i].memaction[j].mask;
            v.trace_tbl[i].memaction[j].memaddr = r.trace_tbl[i].memaction[j].memaddr;
            v.trace_tbl[i].memaction[j].data = r.trace_tbl[i].memaction[j].data;
            v.trace_tbl[i].memaction[j].regaddr = r.trace_tbl[i].memaction[j].regaddr;
            v.trace_tbl[i].memaction[j].complete = r.trace_tbl[i].memaction[j].complete;
            v.trace_tbl[i].memaction[j].sc_release = r.trace_tbl[i].memaction[j].sc_release;
            v.trace_tbl[i].memaction[j].ignored = r.trace_tbl[i].memaction[j].ignored;
        end
        v.trace_tbl[i].completed = r.trace_tbl[i].completed;
    end
    v.tr_wcnt = r.tr_wcnt;
    v.tr_rcnt = r.tr_rcnt;
    v.tr_total = r.tr_total;
    v.tr_opened = r.tr_opened;

    wcnt = int'(r.tr_wcnt);
    rcnt = int'(r.tr_rcnt);
    regcnt = int'(r.trace_tbl[wcnt].regactioncnt);
    memcnt = int'(r.trace_tbl[wcnt].memactioncnt);

    tr_wcnt_nxt = (r.tr_wcnt + 1);
    if (i_e_valid == 1'b1) begin
        v.trace_tbl[wcnt].exec_cnt = (i_dbg_executed_cnt + 1);
        v.trace_tbl[wcnt].pc = i_e_pc;
        v.trace_tbl[wcnt].instr = i_e_instr;

        v.tr_wcnt = (r.tr_wcnt + 1);
        // Clear next element:
        v.trace_tbl[tr_wcnt_nxt].regactioncnt = '0;
        v.trace_tbl[tr_wcnt_nxt].memactioncnt = '0;
        v.trace_tbl[tr_wcnt_nxt].completed = '0;
    end

    if ((i_e_memop_valid == 1'b1) && (i_m_memop_ready == 1'b1)) begin
        v.trace_tbl[wcnt].memactioncnt = (r.trace_tbl[wcnt].memactioncnt + 1);
        v.trace_tbl[wcnt].memaction[memcnt].store = i_e_memop_type[MemopType_Store];
        v.trace_tbl[wcnt].memaction[memcnt].memaddr = i_e_memop_addr;
        v.trace_tbl[wcnt].memaction[memcnt].size = i_e_memop_size;
        // Compute and save mask bit
        mskoff = '0;
        mask = '1;
        mskoff[int'(i_e_memop_size)] = 1'h1;
        mskoff = {mskoff, 3'h0};
        if (mskoff < 64) begin
            mask = '0;
            mask[int'(mskoff)] = 1'h1;
            mask = (mask - 1);
        end
        v.trace_tbl[wcnt].memaction[memcnt].mask = mask;
        v.trace_tbl[wcnt].memaction[memcnt].data = (i_e_memop_wdata & mask);
        v.trace_tbl[wcnt].memaction[memcnt].regaddr = i_e_waddr;
        v.trace_tbl[wcnt].memaction[memcnt].ignored = '0;
        v.trace_tbl[wcnt].memaction[memcnt].complete = '0;
        if (((|i_e_waddr) == 1'b0)
                || ((i_e_memop_type[MemopType_Store] == 1'b1)
                        && (i_e_memop_type[MemopType_Release] == 1'b0))) begin
            v.trace_tbl[wcnt].memaction[memcnt].complete = 1'h1;
        end
        v.trace_tbl[wcnt].memaction[memcnt].sc_release = i_e_memop_type[MemopType_Release];
    end

    if (i_e_wena == 1'b1) begin
        // Direct register writting if it is not a Load operation
        v.trace_tbl[wcnt].regactioncnt = (r.trace_tbl[wcnt].regactioncnt + 1);
        v.trace_tbl[wcnt].regaction[regcnt].waddr = i_e_waddr;
        v.trace_tbl[wcnt].regaction[regcnt].wres = i_e_wdata;
    end else if (i_m_wena == 1'b1) begin
        // Update current rd memory action (memory operations are strictly ordered)
        // Find next instruction with the unfinished memory operation
        checked = 1'b0;
        rcnt_inc = r.tr_rcnt;
        while ((checked == 1'b0) && (rcnt_inc != r.tr_wcnt)) begin
            xcnt = int'(rcnt_inc);
            regcnt = int'(r.trace_tbl[xcnt].regactioncnt);
            for (int i = 0; i < int'(r.trace_tbl[xcnt].memactioncnt); i++) begin
                if ((checked == 1'b0) && (r.trace_tbl[xcnt].memaction[i].complete == 1'b0)) begin
                    checked = 1'b1;
                    v.trace_tbl[xcnt].memaction[i].complete = 1'h1;
                    v.trace_tbl[xcnt].memaction[i].ignored = i_reg_ignored;
                    if (r.trace_tbl[xcnt].memaction[i].sc_release == 1'b1) begin
                        if (i_m_wdata == 64'h0000000000000001) begin
                            v.trace_tbl[xcnt].memaction[i].ignored = 1'h1;
                        end
                        // do not re-write stored value by returning error status
                    end else begin
                        v.trace_tbl[xcnt].memaction[i].data = i_m_wdata;
                    end

                    if (i_reg_ignored == 1'b0) begin
                        v.trace_tbl[xcnt].regaction[regcnt].waddr = i_m_waddr;
                        v.trace_tbl[xcnt].regaction[regcnt].wres = i_m_wdata;
                        v.trace_tbl[xcnt].regactioncnt = (regcnt + 1);
                    end
                end
            end
            if (checked == 1'b0) begin
                rcnt_inc = (rcnt_inc + 1);
            end
        end
    end

    // check instruction data completness
    entry_valid = 1'b1;
    rcnt_inc = r.tr_rcnt;
    outstr = "";
    while ((entry_valid == 1'b1) && (rcnt_inc != r.tr_wcnt)) begin
        for (int i = 0; i < int'(r.trace_tbl[rcnt_inc].memactioncnt); i++) begin
            if (r.trace_tbl[rcnt_inc].memaction[i].complete == 1'b0) begin
                entry_valid = 1'b0;
            end
        end
        if (entry_valid == 1'b1) begin
            tracestr = TraceOutput(rcnt_inc);
            outstr += tracestr;
            rcnt_inc = (rcnt_inc + 1);
        end
    end
    v.tr_rcnt = rcnt_inc;

    if (~async_reset && i_nrst == 1'b0) begin
        for (int i = 0; i < TRACE_TBL_SZ; i++) begin
            v.trace_tbl[i].exec_cnt = 64'h0000000000000000;
            v.trace_tbl[i].pc = 64'h0000000000000000;
            v.trace_tbl[i].instr = 32'h00000000;
            v.trace_tbl[i].regactioncnt = 32'h00000000;
            v.trace_tbl[i].memactioncnt = 32'h00000000;
            for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                v.trace_tbl[i].regaction[j].waddr = 6'h00;
                v.trace_tbl[i].regaction[j].wres = 64'h0000000000000000;
            end
            for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                v.trace_tbl[i].memaction[j].store = 1'h0;
                v.trace_tbl[i].memaction[j].size = 2'h0;
                v.trace_tbl[i].memaction[j].mask = 64'h0000000000000000;
                v.trace_tbl[i].memaction[j].memaddr = 64'h0000000000000000;
                v.trace_tbl[i].memaction[j].data = 64'h0000000000000000;
                v.trace_tbl[i].memaction[j].regaddr = 6'h00;
                v.trace_tbl[i].memaction[j].complete = 1'h0;
                v.trace_tbl[i].memaction[j].sc_release = 1'h0;
                v.trace_tbl[i].memaction[j].ignored = 1'h0;
            end
            v.trace_tbl[i].completed = 1'h0;
        end
        v.tr_wcnt = 6'h00;
        v.tr_rcnt = 6'h00;
        v.tr_total = 6'h00;
        v.tr_opened = 6'h00;
    end

    for (int i = 0; i < TRACE_TBL_SZ; i++) begin
        rin.trace_tbl[i].exec_cnt = v.trace_tbl[i].exec_cnt;
        rin.trace_tbl[i].pc = v.trace_tbl[i].pc;
        rin.trace_tbl[i].instr = v.trace_tbl[i].instr;
        rin.trace_tbl[i].regactioncnt = v.trace_tbl[i].regactioncnt;
        rin.trace_tbl[i].memactioncnt = v.trace_tbl[i].memactioncnt;
        for (int j = 0; j < TRACE_TBL_SZ; j++) begin
            rin.trace_tbl[i].regaction[j].waddr = v.trace_tbl[i].regaction[j].waddr;
            rin.trace_tbl[i].regaction[j].wres = v.trace_tbl[i].regaction[j].wres;
        end
        for (int j = 0; j < TRACE_TBL_SZ; j++) begin
            rin.trace_tbl[i].memaction[j].store = v.trace_tbl[i].memaction[j].store;
            rin.trace_tbl[i].memaction[j].size = v.trace_tbl[i].memaction[j].size;
            rin.trace_tbl[i].memaction[j].mask = v.trace_tbl[i].memaction[j].mask;
            rin.trace_tbl[i].memaction[j].memaddr = v.trace_tbl[i].memaction[j].memaddr;
            rin.trace_tbl[i].memaction[j].data = v.trace_tbl[i].memaction[j].data;
            rin.trace_tbl[i].memaction[j].regaddr = v.trace_tbl[i].memaction[j].regaddr;
            rin.trace_tbl[i].memaction[j].complete = v.trace_tbl[i].memaction[j].complete;
            rin.trace_tbl[i].memaction[j].sc_release = v.trace_tbl[i].memaction[j].sc_release;
            rin.trace_tbl[i].memaction[j].ignored = v.trace_tbl[i].memaction[j].ignored;
        end
        rin.trace_tbl[i].completed = v.trace_tbl[i].completed;
    end
    rin.tr_wcnt = v.tr_wcnt;
    rin.tr_rcnt = v.tr_rcnt;
    rin.tr_total = v.tr_total;
    rin.tr_opened = v.tr_opened;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                for (int i = 0; i < TRACE_TBL_SZ; i++) begin
                    r.trace_tbl[i].exec_cnt <= 64'h0000000000000000;
                    r.trace_tbl[i].pc <= 64'h0000000000000000;
                    r.trace_tbl[i].instr <= 32'h00000000;
                    r.trace_tbl[i].regactioncnt <= 32'h00000000;
                    r.trace_tbl[i].memactioncnt <= 32'h00000000;
                    for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                        r.trace_tbl[i].regaction[j].waddr <= 6'h00;
                        r.trace_tbl[i].regaction[j].wres <= 64'h0000000000000000;
                    end
                    for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                        r.trace_tbl[i].memaction[j].store <= 1'h0;
                        r.trace_tbl[i].memaction[j].size <= 2'h0;
                        r.trace_tbl[i].memaction[j].mask <= 64'h0000000000000000;
                        r.trace_tbl[i].memaction[j].memaddr <= 64'h0000000000000000;
                        r.trace_tbl[i].memaction[j].data <= 64'h0000000000000000;
                        r.trace_tbl[i].memaction[j].regaddr <= 6'h00;
                        r.trace_tbl[i].memaction[j].complete <= 1'h0;
                        r.trace_tbl[i].memaction[j].sc_release <= 1'h0;
                        r.trace_tbl[i].memaction[j].ignored <= 1'h0;
                    end
                    r.trace_tbl[i].completed <= 1'h0;
                end
                r.tr_wcnt <= 6'h00;
                r.tr_rcnt <= 6'h00;
                r.tr_total <= 6'h00;
                r.tr_opened <= 6'h00;
            end else begin
                for (int i = 0; i < TRACE_TBL_SZ; i++) begin
                    r.trace_tbl[i].exec_cnt <= rin.trace_tbl[i].exec_cnt;
                    r.trace_tbl[i].pc <= rin.trace_tbl[i].pc;
                    r.trace_tbl[i].instr <= rin.trace_tbl[i].instr;
                    r.trace_tbl[i].regactioncnt <= rin.trace_tbl[i].regactioncnt;
                    r.trace_tbl[i].memactioncnt <= rin.trace_tbl[i].memactioncnt;
                    for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                        r.trace_tbl[i].regaction[j].waddr <= rin.trace_tbl[i].regaction[j].waddr;
                        r.trace_tbl[i].regaction[j].wres <= rin.trace_tbl[i].regaction[j].wres;
                    end
                    for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                        r.trace_tbl[i].memaction[j].store <= rin.trace_tbl[i].memaction[j].store;
                        r.trace_tbl[i].memaction[j].size <= rin.trace_tbl[i].memaction[j].size;
                        r.trace_tbl[i].memaction[j].mask <= rin.trace_tbl[i].memaction[j].mask;
                        r.trace_tbl[i].memaction[j].memaddr <= rin.trace_tbl[i].memaction[j].memaddr;
                        r.trace_tbl[i].memaction[j].data <= rin.trace_tbl[i].memaction[j].data;
                        r.trace_tbl[i].memaction[j].regaddr <= rin.trace_tbl[i].memaction[j].regaddr;
                        r.trace_tbl[i].memaction[j].complete <= rin.trace_tbl[i].memaction[j].complete;
                        r.trace_tbl[i].memaction[j].sc_release <= rin.trace_tbl[i].memaction[j].sc_release;
                        r.trace_tbl[i].memaction[j].ignored <= rin.trace_tbl[i].memaction[j].ignored;
                    end
                    r.trace_tbl[i].completed <= rin.trace_tbl[i].completed;
                end
                r.tr_wcnt <= rin.tr_wcnt;
                r.tr_rcnt <= rin.tr_rcnt;
                r.tr_total <= rin.tr_total;
                r.tr_opened <= rin.tr_opened;
            end

            if (outstr != "") begin
                $fwrite(fl, "%s", outstr);
            end
            outstr = "";
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            for (int i = 0; i < TRACE_TBL_SZ; i++) begin
                r.trace_tbl[i].exec_cnt <= rin.trace_tbl[i].exec_cnt;
                r.trace_tbl[i].pc <= rin.trace_tbl[i].pc;
                r.trace_tbl[i].instr <= rin.trace_tbl[i].instr;
                r.trace_tbl[i].regactioncnt <= rin.trace_tbl[i].regactioncnt;
                r.trace_tbl[i].memactioncnt <= rin.trace_tbl[i].memactioncnt;
                for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                    r.trace_tbl[i].regaction[j].waddr <= rin.trace_tbl[i].regaction[j].waddr;
                    r.trace_tbl[i].regaction[j].wres <= rin.trace_tbl[i].regaction[j].wres;
                end
                for (int j = 0; j < TRACE_TBL_SZ; j++) begin
                    r.trace_tbl[i].memaction[j].store <= rin.trace_tbl[i].memaction[j].store;
                    r.trace_tbl[i].memaction[j].size <= rin.trace_tbl[i].memaction[j].size;
                    r.trace_tbl[i].memaction[j].mask <= rin.trace_tbl[i].memaction[j].mask;
                    r.trace_tbl[i].memaction[j].memaddr <= rin.trace_tbl[i].memaction[j].memaddr;
                    r.trace_tbl[i].memaction[j].data <= rin.trace_tbl[i].memaction[j].data;
                    r.trace_tbl[i].memaction[j].regaddr <= rin.trace_tbl[i].memaction[j].regaddr;
                    r.trace_tbl[i].memaction[j].complete <= rin.trace_tbl[i].memaction[j].complete;
                    r.trace_tbl[i].memaction[j].sc_release <= rin.trace_tbl[i].memaction[j].sc_release;
                    r.trace_tbl[i].memaction[j].ignored <= rin.trace_tbl[i].memaction[j].ignored;
                end
                r.trace_tbl[i].completed <= rin.trace_tbl[i].completed;
            end
            r.tr_wcnt <= rin.tr_wcnt;
            r.tr_rcnt <= rin.tr_rcnt;
            r.tr_total <= rin.tr_total;
            r.tr_opened <= rin.tr_opened;

            if (outstr != "") begin
                $fwrite(fl, "%s", outstr);
            end
            outstr = "";
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: Tracer
