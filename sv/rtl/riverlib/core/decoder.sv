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

module InstrDecoder #(
    parameter bit async_reset = 1'b0,
    parameter bit fpu_ena = 1'b1
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_f_pc,     // Fetched pc
    input logic [63:0] i_f_instr,                           // Fetched instruction value
    input logic i_instr_load_fault,                         // fault instruction's address
    input logic i_instr_page_fault_x,                       // Instruction MMU page fault
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_e_npc,    // executor expected instr pointer
    output logic [5:0] o_radr1,                             // register bank address 1 (rs1)
    output logic [5:0] o_radr2,                             // register bank address 2 (rs2)
    output logic [5:0] o_waddr,                             // register bank output (rd)
    output logic [11:0] o_csr_addr,                         // CSR bank output
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_imm,     // immediate constant decoded from instruction
    input logic i_flush_pipeline,                           // reset pipeline and cache
    input logic i_progbuf_ena,                              // executing from progbuf
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pc,      // Current instruction pointer value
    output logic [31:0] o_instr,                            // Current instruction value
    output logic o_memop_store,                             // Store to memory operation
    output logic o_memop_load,                              // Load from memoru operation
    output logic o_memop_sign_ext,                          // Load memory value with sign extending
    output logic [1:0] o_memop_size,                        // Memory transaction size
    output logic o_rv32,                                    // 32-bits instruction
    output logic o_compressed,                              // C-type instruction
    output logic o_amo,                                     // A-type instruction
    output logic o_f64,                                     // 64-bits FPU (D-extension)
    output logic o_unsigned_op,                             // Unsigned operands
    output logic [river_cfg_pkg::ISA_Total-1:0] o_isa_type, // Instruction format accordingly with ISA
    output logic [river_cfg_pkg::Instr_Total-1:0] o_instr_vec,// One bit per decoded instruction bus
    output logic o_exception,                               // Exception detected
    output logic o_instr_load_fault,                        // fault instruction's address
    output logic o_instr_page_fault_x,                      // IMMU page fault signal
    output logic o_progbuf_ena
);

import river_cfg_pkg::*;
import decoder_pkg::*;

DecoderDataType wd[0: (FULL_DEC_DEPTH + DEC_BLOCK) - 1];
logic [RISCV_ARCH-1:0] wb_f_pc[0: DEC_NUM - 1];
logic [31:0] wb_f_instr[0: DEC_NUM - 1];
InstrDecoder_registers r, rin;

for (genvar i = 0; i < DEC_NUM; i++) begin: rvx
    DecoderRv #(
        .async_reset(async_reset),
        .fpu_ena(fpu_ena)
    ) rv (
        .i_clk(i_clk),
        .i_nrst(i_nrst),
        .i_flush_pipeline(i_flush_pipeline),
        .i_progbuf_ena(i_progbuf_ena),
        .i_f_pc(wb_f_pc[i]),
        .i_f_instr(wb_f_instr[i]),
        .i_instr_load_fault(i_instr_load_fault),
        .i_instr_page_fault_x(i_instr_page_fault_x),
        .o_radr1(wd[(2 * i)].radr1),
        .o_radr2(wd[(2 * i)].radr2),
        .o_waddr(wd[(2 * i)].waddr),
        .o_csr_addr(wd[(2 * i)].csr_addr),
        .o_imm(wd[(2 * i)].imm),
        .o_pc(wd[(2 * i)].pc),
        .o_instr(wd[(2 * i)].instr),
        .o_memop_store(wd[(2 * i)].memop_store),
        .o_memop_load(wd[(2 * i)].memop_load),
        .o_memop_sign_ext(wd[(2 * i)].memop_sign_ext),
        .o_memop_size(wd[(2 * i)].memop_size),
        .o_rv32(wd[(2 * i)].rv32),
        .o_compressed(wd[(2 * i)].compressed),
        .o_amo(wd[(2 * i)].amo),
        .o_f64(wd[(2 * i)].f64),
        .o_unsigned_op(wd[(2 * i)].unsigned_op),
        .o_isa_type(wd[(2 * i)].isa_type),
        .o_instr_vec(wd[(2 * i)].instr_vec),
        .o_exception(wd[(2 * i)].instr_unimplemented),
        .o_instr_load_fault(wd[(2 * i)].instr_load_fault),
        .o_instr_page_fault_x(wd[(2 * i)].instr_page_fault_x),
        .o_progbuf_ena(wd[(2 * i)].progbuf_ena)
    );

end: rvx

for (genvar i = 0; i < DEC_NUM; i++) begin: rvcx
    DecoderRvc #(
        .async_reset(async_reset)
    ) rvc (
        .i_clk(i_clk),
        .i_nrst(i_nrst),
        .i_flush_pipeline(i_flush_pipeline),
        .i_progbuf_ena(i_progbuf_ena),
        .i_f_pc(wb_f_pc[i]),
        .i_f_instr(wb_f_instr[i]),
        .i_instr_load_fault(i_instr_load_fault),
        .i_instr_page_fault_x(i_instr_page_fault_x),
        .o_radr1(wd[((2 * i) + 1)].radr1),
        .o_radr2(wd[((2 * i) + 1)].radr2),
        .o_waddr(wd[((2 * i) + 1)].waddr),
        .o_csr_addr(wd[((2 * i) + 1)].csr_addr),
        .o_imm(wd[((2 * i) + 1)].imm),
        .o_pc(wd[((2 * i) + 1)].pc),
        .o_instr(wd[((2 * i) + 1)].instr),
        .o_memop_store(wd[((2 * i) + 1)].memop_store),
        .o_memop_load(wd[((2 * i) + 1)].memop_load),
        .o_memop_sign_ext(wd[((2 * i) + 1)].memop_sign_ext),
        .o_memop_size(wd[((2 * i) + 1)].memop_size),
        .o_rv32(wd[((2 * i) + 1)].rv32),
        .o_compressed(wd[((2 * i) + 1)].compressed),
        .o_amo(wd[((2 * i) + 1)].amo),
        .o_f64(wd[((2 * i) + 1)].f64),
        .o_unsigned_op(wd[((2 * i) + 1)].unsigned_op),
        .o_isa_type(wd[((2 * i) + 1)].isa_type),
        .o_instr_vec(wd[((2 * i) + 1)].instr_vec),
        .o_exception(wd[((2 * i) + 1)].instr_unimplemented),
        .o_instr_load_fault(wd[((2 * i) + 1)].instr_load_fault),
        .o_instr_page_fault_x(wd[((2 * i) + 1)].instr_page_fault_x),
        .o_progbuf_ena(wd[((2 * i) + 1)].progbuf_ena)
    );

end: rvcx

always_comb
begin: comb_proc
    InstrDecoder_registers v;
    int selidx;
    logic shift_ena;

    selidx = 0;
    shift_ena = 0;

    for (int i = 0; i < FULL_DEC_DEPTH; i++) begin
        v.d[i].pc = r.d[i].pc;
        v.d[i].isa_type = r.d[i].isa_type;
        v.d[i].instr_vec = r.d[i].instr_vec;
        v.d[i].instr = r.d[i].instr;
        v.d[i].memop_store = r.d[i].memop_store;
        v.d[i].memop_load = r.d[i].memop_load;
        v.d[i].memop_sign_ext = r.d[i].memop_sign_ext;
        v.d[i].memop_size = r.d[i].memop_size;
        v.d[i].unsigned_op = r.d[i].unsigned_op;
        v.d[i].rv32 = r.d[i].rv32;
        v.d[i].f64 = r.d[i].f64;
        v.d[i].compressed = r.d[i].compressed;
        v.d[i].amo = r.d[i].amo;
        v.d[i].instr_load_fault = r.d[i].instr_load_fault;
        v.d[i].instr_page_fault_x = r.d[i].instr_page_fault_x;
        v.d[i].instr_unimplemented = r.d[i].instr_unimplemented;
        v.d[i].radr1 = r.d[i].radr1;
        v.d[i].radr2 = r.d[i].radr2;
        v.d[i].waddr = r.d[i].waddr;
        v.d[i].csr_addr = r.d[i].csr_addr;
        v.d[i].imm = r.d[i].imm;
        v.d[i].progbuf_ena = r.d[i].progbuf_ena;
    end

    for (int i = 0; i < FULL_DEC_DEPTH; i++) begin
        wd[(DEC_BLOCK + i)] = r.d[i];
    end

    if (i_f_pc != wd[0].pc) begin
        shift_ena = 1'b1;
    end

    // Shift decoder buffer when new instruction available
    if (shift_ena == 1'b1) begin
        for (int i = 0; i < DEC_BLOCK; i++) begin
            v.d[i] = wd[i];
        end
        for (int i = DEC_BLOCK; i < FULL_DEC_DEPTH; i++) begin
            v.d[i] = r.d[(i - DEC_BLOCK)];
        end
    end

    // Select output decoder:
    for (int i = 0; i < ((FULL_DEC_DEPTH + DEC_BLOCK) / 2); i++) begin
        if (i_e_npc == wd[(2 * i)].pc) begin
            if (wd[(2 * i)].compressed == 1'b0) begin
                selidx = (2 * i);
            end else begin
                selidx = ((2 * i) + 1);
            end
        end
    end

    // generate decoders inputs with offset
    for (int i = 0; i < DEC_NUM; i++) begin
        wb_f_pc[i] = (i_f_pc + (2 * i));
        wb_f_instr[i] = i_f_instr[(16 * i) +: 32];
    end

    if ((~async_reset && i_nrst == 1'b0) || (i_flush_pipeline == 1'b1)) begin
        for (int i = 0; i < FULL_DEC_DEPTH; i++) begin
            v.d[i].pc = '1;
            v.d[i].isa_type = 6'h00;
            v.d[i].instr_vec = '0;
            v.d[i].instr = '1;
            v.d[i].memop_store = 1'h0;
            v.d[i].memop_load = 1'h0;
            v.d[i].memop_sign_ext = 1'h0;
            v.d[i].memop_size = MEMOP_1B;
            v.d[i].unsigned_op = 1'h0;
            v.d[i].rv32 = 1'h0;
            v.d[i].f64 = 1'h0;
            v.d[i].compressed = 1'h0;
            v.d[i].amo = 1'h0;
            v.d[i].instr_load_fault = 1'h0;
            v.d[i].instr_page_fault_x = 1'h0;
            v.d[i].instr_unimplemented = 1'h0;
            v.d[i].radr1 = 6'h00;
            v.d[i].radr2 = 6'h00;
            v.d[i].waddr = 6'h00;
            v.d[i].csr_addr = 12'h000;
            v.d[i].imm = 64'h0000000000000000;
            v.d[i].progbuf_ena = 1'h0;
        end
    end

    o_pc = wd[selidx].pc;
    o_instr = wd[selidx].instr;
    o_memop_load = wd[selidx].memop_load;
    o_memop_store = wd[selidx].memop_store;
    o_memop_sign_ext = wd[selidx].memop_sign_ext;
    o_memop_size = wd[selidx].memop_size;
    o_unsigned_op = wd[selidx].unsigned_op;
    o_rv32 = wd[selidx].rv32;
    o_f64 = wd[selidx].f64;
    o_compressed = wd[selidx].compressed;
    o_amo = wd[selidx].amo;
    o_isa_type = wd[selidx].isa_type;
    o_instr_vec = wd[selidx].instr_vec;
    o_exception = wd[selidx].instr_unimplemented;
    o_instr_load_fault = wd[selidx].instr_load_fault;
    o_instr_page_fault_x = wd[selidx].instr_page_fault_x;
    o_radr1 = wd[selidx].radr1;
    o_radr2 = wd[selidx].radr2;
    o_waddr = wd[selidx].waddr;
    o_csr_addr = wd[selidx].csr_addr;
    o_imm = wd[selidx].imm;
    o_progbuf_ena = wd[selidx].progbuf_ena;

    for (int i = 0; i < FULL_DEC_DEPTH; i++) begin
        rin.d[i].pc = v.d[i].pc;
        rin.d[i].isa_type = v.d[i].isa_type;
        rin.d[i].instr_vec = v.d[i].instr_vec;
        rin.d[i].instr = v.d[i].instr;
        rin.d[i].memop_store = v.d[i].memop_store;
        rin.d[i].memop_load = v.d[i].memop_load;
        rin.d[i].memop_sign_ext = v.d[i].memop_sign_ext;
        rin.d[i].memop_size = v.d[i].memop_size;
        rin.d[i].unsigned_op = v.d[i].unsigned_op;
        rin.d[i].rv32 = v.d[i].rv32;
        rin.d[i].f64 = v.d[i].f64;
        rin.d[i].compressed = v.d[i].compressed;
        rin.d[i].amo = v.d[i].amo;
        rin.d[i].instr_load_fault = v.d[i].instr_load_fault;
        rin.d[i].instr_page_fault_x = v.d[i].instr_page_fault_x;
        rin.d[i].instr_unimplemented = v.d[i].instr_unimplemented;
        rin.d[i].radr1 = v.d[i].radr1;
        rin.d[i].radr2 = v.d[i].radr2;
        rin.d[i].waddr = v.d[i].waddr;
        rin.d[i].csr_addr = v.d[i].csr_addr;
        rin.d[i].imm = v.d[i].imm;
        rin.d[i].progbuf_ena = v.d[i].progbuf_ena;
    end
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                for (int i = 0; i < FULL_DEC_DEPTH; i++) begin
                    r.d[i].pc <= '1;
                    r.d[i].isa_type <= 6'h00;
                    r.d[i].instr_vec <= '0;
                    r.d[i].instr <= '1;
                    r.d[i].memop_store <= 1'h0;
                    r.d[i].memop_load <= 1'h0;
                    r.d[i].memop_sign_ext <= 1'h0;
                    r.d[i].memop_size <= MEMOP_1B;
                    r.d[i].unsigned_op <= 1'h0;
                    r.d[i].rv32 <= 1'h0;
                    r.d[i].f64 <= 1'h0;
                    r.d[i].compressed <= 1'h0;
                    r.d[i].amo <= 1'h0;
                    r.d[i].instr_load_fault <= 1'h0;
                    r.d[i].instr_page_fault_x <= 1'h0;
                    r.d[i].instr_unimplemented <= 1'h0;
                    r.d[i].radr1 <= 6'h00;
                    r.d[i].radr2 <= 6'h00;
                    r.d[i].waddr <= 6'h00;
                    r.d[i].csr_addr <= 12'h000;
                    r.d[i].imm <= 64'h0000000000000000;
                    r.d[i].progbuf_ena <= 1'h0;
                end
            end else begin
                for (int i = 0; i < FULL_DEC_DEPTH; i++) begin
                    r.d[i].pc <= rin.d[i].pc;
                    r.d[i].isa_type <= rin.d[i].isa_type;
                    r.d[i].instr_vec <= rin.d[i].instr_vec;
                    r.d[i].instr <= rin.d[i].instr;
                    r.d[i].memop_store <= rin.d[i].memop_store;
                    r.d[i].memop_load <= rin.d[i].memop_load;
                    r.d[i].memop_sign_ext <= rin.d[i].memop_sign_ext;
                    r.d[i].memop_size <= rin.d[i].memop_size;
                    r.d[i].unsigned_op <= rin.d[i].unsigned_op;
                    r.d[i].rv32 <= rin.d[i].rv32;
                    r.d[i].f64 <= rin.d[i].f64;
                    r.d[i].compressed <= rin.d[i].compressed;
                    r.d[i].amo <= rin.d[i].amo;
                    r.d[i].instr_load_fault <= rin.d[i].instr_load_fault;
                    r.d[i].instr_page_fault_x <= rin.d[i].instr_page_fault_x;
                    r.d[i].instr_unimplemented <= rin.d[i].instr_unimplemented;
                    r.d[i].radr1 <= rin.d[i].radr1;
                    r.d[i].radr2 <= rin.d[i].radr2;
                    r.d[i].waddr <= rin.d[i].waddr;
                    r.d[i].csr_addr <= rin.d[i].csr_addr;
                    r.d[i].imm <= rin.d[i].imm;
                    r.d[i].progbuf_ena <= rin.d[i].progbuf_ena;
                end
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            for (int i = 0; i < FULL_DEC_DEPTH; i++) begin
                r.d[i].pc <= rin.d[i].pc;
                r.d[i].isa_type <= rin.d[i].isa_type;
                r.d[i].instr_vec <= rin.d[i].instr_vec;
                r.d[i].instr <= rin.d[i].instr;
                r.d[i].memop_store <= rin.d[i].memop_store;
                r.d[i].memop_load <= rin.d[i].memop_load;
                r.d[i].memop_sign_ext <= rin.d[i].memop_sign_ext;
                r.d[i].memop_size <= rin.d[i].memop_size;
                r.d[i].unsigned_op <= rin.d[i].unsigned_op;
                r.d[i].rv32 <= rin.d[i].rv32;
                r.d[i].f64 <= rin.d[i].f64;
                r.d[i].compressed <= rin.d[i].compressed;
                r.d[i].amo <= rin.d[i].amo;
                r.d[i].instr_load_fault <= rin.d[i].instr_load_fault;
                r.d[i].instr_page_fault_x <= rin.d[i].instr_page_fault_x;
                r.d[i].instr_unimplemented <= rin.d[i].instr_unimplemented;
                r.d[i].radr1 <= rin.d[i].radr1;
                r.d[i].radr2 <= rin.d[i].radr2;
                r.d[i].waddr <= rin.d[i].waddr;
                r.d[i].csr_addr <= rin.d[i].csr_addr;
                r.d[i].imm <= rin.d[i].imm;
                r.d[i].progbuf_ena <= rin.d[i].progbuf_ena;
            end
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: InstrDecoder
