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

module InstrExecute #(
    parameter bit async_reset = 1'b0,
    parameter bit fpu_ena = 1'b1
)
(
    input logic i_clk,                                      // CPU clock
    input logic i_nrst,                                     // Reset: active LOW
    input logic [5:0] i_d_radr1,                            // rs1 address
    input logic [5:0] i_d_radr2,                            // rs2 address
    input logic [5:0] i_d_waddr,                            // rd address
    input logic [11:0] i_d_csr_addr,                        // decoded CSR address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_d_imm,    // immediate value
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_d_pc,     // Instruction pointer on decoded instruction
    input logic [31:0] i_d_instr,                           // Decoded instruction value
    input logic i_d_progbuf_ena,                            // instruction from progbuf passed decoder
    input logic [5:0] i_wb_waddr,                           // write back address
    input logic i_memop_store,                              // Store to memory operation
    input logic i_memop_load,                               // Load from memoru operation
    input logic i_memop_sign_ext,                           // Load memory value with sign extending
    input logic [1:0] i_memop_size,                         // Memory transaction size
    input logic i_unsigned_op,                              // Unsigned operands
    input logic i_rv32,                                     // 32-bits instruction
    input logic i_compressed,                               // C-extension (2-bytes length)
    input logic i_amo,                                      // A-extension (atomic)
    input logic i_f64,                                      // D-extension (FPU)
    input logic [river_cfg_pkg::ISA_Total-1:0] i_isa_type,  // Type of the instruction's structure (ISA spec.)
    input logic [river_cfg_pkg::Instr_Total-1:0] i_ivec,    // One pulse per supported instruction.
    input logic i_stack_overflow,                           // exception stack overflow
    input logic i_stack_underflow,                          // exception stack overflow
    input logic i_unsup_exception,                          // Unsupported instruction exception
    input logic i_instr_load_fault,                         // fault instruction's address. Bus returned ERR on read transaction
    input logic i_mem_valid,                                // memory operation done (need for AMO)
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_mem_rdata,// memory operation read data (need for AMO)
    input logic i_mem_ex_debug,                             // Memoryaccess: Debug requested processed with error. Ignore it.
    input logic i_mem_ex_load_fault,                        // Memoryaccess: Bus response with SLVERR or DECERR on read data
    input logic i_mem_ex_store_fault,                       // Memoryaccess: Bus response with SLVERR or DECERR on write data
    input logic i_page_fault_x,                             // IMMU execute page fault signal
    input logic i_page_fault_r,                             // DMMU read access page fault
    input logic i_page_fault_w,                             // DMMU write access page fault
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_mem_ex_addr,// Memoryaccess: exception address
    input logic [river_cfg_pkg::IRQ_TOTAL-1:0] i_irq_pending,// Per Hart pending interrupts pins
    input logic i_wakeup,                                   // There's pending bit even if interrupts globally disabled
    input logic i_haltreq,                                  // halt request from debug unit
    input logic i_resumereq,                                // resume request from debug unit
    input logic i_step,                                     // resume with step
    input logic i_dbg_progbuf_ena,                          // progbuf mode enabled
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_rdata1,   // Integer/Float register value 1
    input logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] i_rtag1,
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_rdata2,   // Integer/Float register value 2
    input logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] i_rtag2,
    output logic [5:0] o_radr1,
    output logic [5:0] o_radr2,
    output logic o_reg_wena,
    output logic [5:0] o_reg_waddr,                         // Address to store result of the instruction (0=do not store)
    output logic [river_cfg_pkg::CFG_REG_TAG_WIDTH-1:0] o_reg_wtag,
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_reg_wdata,// Value to store
    output logic o_csr_req_valid,                           // Access to CSR request
    input logic i_csr_req_ready,                            // CSR module is ready to accept request
    output logic [river_cfg_pkg::CsrReq_TotalBits-1:0] o_csr_req_type,// Request type: [0]-read csr; [1]-write csr; [2]-change mode
    output logic [11:0] o_csr_req_addr,                     // Requested CSR address
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_csr_req_data,// CSR new value
    input logic i_csr_resp_valid,                           // CSR module Response is valid
    output logic o_csr_resp_ready,                          // Executor is ready to accept response
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_csr_resp_data,// Responded CSR data
    input logic i_csr_resp_exception,                       // Raise exception on CSR access
    output logic o_memop_valid,                             // Request to memory is valid
    output logic o_memop_debug,                             // Debug Request shouldn't modify registers in write back stage
    output logic o_memop_sign_ext,                          // Load data with sign extending
    output logic [river_cfg_pkg::MemopType_Total-1:0] o_memop_type,// [0]: 1=store/0=Load data
    output logic [1:0] o_memop_size,                        // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_memop_memaddr,// Memory access address
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_memop_wdata,
    input logic i_memop_ready,                              // memaccess is ready to accept memop on next clock
    input logic i_memop_idle,                               // No memory operations in progress
    input logic i_dbg_mem_req_valid,                        // Debug Request to memory is valid
    input logic i_dbg_mem_req_write,                        // 0=read; 1=write
    input logic [1:0] i_dbg_mem_req_size,                   // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dbg_mem_req_addr,// Memory access address
    input logic [river_cfg_pkg::RISCV_ARCH-1:0] i_dbg_mem_req_wdata,
    output logic o_dbg_mem_req_ready,                       // Debug emmory request was accepted
    output logic o_dbg_mem_req_error,                       // Debug memory reques misaliged
    output logic o_valid,                                   // Output is valid
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_pc,      // Valid instruction pointer
    output logic [river_cfg_pkg::RISCV_ARCH-1:0] o_npc,     // Next instruction pointer. Next decoded pc must match to this value or will be ignored.
    output logic [31:0] o_instr,                            // Valid instruction value
    output logic o_call,                                    // CALL pseudo instruction detected
    output logic o_ret,                                     // RET pseudoinstruction detected (hw stack tracing)
    output logic o_jmp,                                     // Jump was executed
    output logic o_halted
);

import river_cfg_pkg::*;
import execute_pkg::*;

select_type wb_select[0: Res_Total - 1];
logic [2:0] wb_alu_mode;
logic [6:0] wb_addsub_mode;
logic [3:0] wb_shifter_mode;
logic w_arith_residual_high;
logic w_mul_hsu;
logic [Instr_FPU_Total-1:0] wb_fpu_vec;
logic w_ex_fpu_invalidop;                                   // FPU Exception: invalid operation
logic w_ex_fpu_divbyzero;                                   // FPU Exception: divide by zero
logic w_ex_fpu_overflow;                                    // FPU Exception: overflow
logic w_ex_fpu_underflow;                                   // FPU Exception: underflow
logic w_ex_fpu_inexact;                                     // FPU Exception: inexact
logic w_hazard1;
logic w_hazard2;
logic [RISCV_ARCH-1:0] wb_rdata1;
logic [RISCV_ARCH-1:0] wb_rdata2;
logic [RISCV_ARCH-1:0] wb_shifter_a1;                       // Shifters operand 1
logic [5:0] wb_shifter_a2;                                  // Shifters operand 2
logic [CFG_REG_TAG_WIDTH-1:0] tag_expected[0: INTREGS_TOTAL - 1];
InstrExecute_registers r, rin;

function logic [3:0] irq2idx(input logic [IRQ_TOTAL-1:0] irqbus);
logic [3:0] ret;
begin
    // see page 34, cursive text about prioirty handling:
    //     1. Higher priv mode must be served first
    //     2. External interrupts first
    //     3. SW interrupts seconds
    //     4. Timer interrupts last
    if (irqbus[IRQ_MEIP] == 1'b1) begin
        ret = IRQ_MEIP;
    end else if (irqbus[IRQ_MSIP] == 1'b1) begin
        ret = IRQ_MSIP;
    end else if (irqbus[IRQ_MTIP] == 1'b1) begin
        ret = IRQ_MTIP;
    end else if (irqbus[IRQ_SEIP] == 1'b1) begin
        ret = IRQ_SEIP;
    end else if (irqbus[IRQ_SSIP] == 1'b1) begin
        ret = IRQ_SSIP;
    end else if (irqbus[IRQ_STIP] == 1'b1) begin
        ret = IRQ_STIP;
    end
    return ret;
end
endfunction: irq2idx

AluLogic #(
    .async_reset(async_reset)
) alu0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mode(wb_alu_mode),
    .i_a1(wb_rdata1),
    .i_a2(wb_rdata2),
    .o_res(wb_select[Res_Alu].res)
);


IntAddSub #(
    .async_reset(async_reset)
) addsub0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mode(wb_addsub_mode),
    .i_a1(wb_rdata1),
    .i_a2(wb_rdata2),
    .o_res(wb_select[Res_AddSub].res)
);


IntMul #(
    .async_reset(async_reset)
) mul0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(wb_select[Res_IMul].ena),
    .i_unsigned(i_unsigned_op),
    .i_hsu(w_mul_hsu),
    .i_high(w_arith_residual_high),
    .i_rv32(i_rv32),
    .i_a1(wb_rdata1),
    .i_a2(wb_rdata2),
    .o_res(wb_select[Res_IMul].res),
    .o_valid(wb_select[Res_IMul].valid)
);


IntDiv #(
    .async_reset(async_reset)
) div0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_ena(wb_select[Res_IDiv].ena),
    .i_unsigned(i_unsigned_op),
    .i_rv32(i_rv32),
    .i_residual(w_arith_residual_high),
    .i_a1(wb_rdata1),
    .i_a2(wb_rdata2),
    .o_res(wb_select[Res_IDiv].res),
    .o_valid(wb_select[Res_IDiv].valid)
);


Shifter #(
    .async_reset(async_reset)
) sh0 (
    .i_clk(i_clk),
    .i_nrst(i_nrst),
    .i_mode(wb_shifter_mode),
    .i_a1(wb_shifter_a1),
    .i_a2(wb_shifter_a2),
    .o_res(wb_select[Res_Shifter].res)
);


generate
    if (fpu_ena) begin: fpu_en
        FpuTop #(
            .async_reset(async_reset)
        ) fpu0 (
            .i_clk(i_clk),
            .i_nrst(i_nrst),
            .i_ena(wb_select[Res_FPU].ena),
            .i_ivec(wb_fpu_vec),
            .i_a(wb_rdata1),
            .i_b(wb_rdata2),
            .o_res(wb_select[Res_FPU].res),
            .o_ex_invalidop(w_ex_fpu_invalidop),
            .o_ex_divbyzero(w_ex_fpu_divbyzero),
            .o_ex_overflow(w_ex_fpu_overflow),
            .o_ex_underflow(w_ex_fpu_underflow),
            .o_ex_inexact(w_ex_fpu_inexact),
            .o_valid(wb_select[Res_FPU].valid)
        );
    end: fpu_en
    else begin: fpu_dis
        assign wb_select[Res_FPU].res = '0;
        assign wb_select[Res_FPU].valid = '0;
        assign w_ex_fpu_invalidop = 1'b0;
        assign w_ex_fpu_divbyzero = 1'b0;
        assign w_ex_fpu_overflow = 1'b0;
        assign w_ex_fpu_underflow = 1'b0;
        assign w_ex_fpu_inexact = 1'b0;
    end: fpu_dis

endgenerate

always_comb
begin: comb_proc
    InstrExecute_registers v;
    logic v_d_valid;
    logic v_csr_req_valid;
    logic v_csr_resp_ready;
    logic [RISCV_ARCH-1:0] vb_csr_cmd_wdata;
    logic [RISCV_ARCH-1:0] vb_res;
    logic [RISCV_ARCH-1:0] vb_prog_npc;
    logic [RISCV_ARCH-1:0] vb_npc_incr;
    logic [RISCV_ARCH-1:0] vb_off;
    logic [RISCV_ARCH-1:0] vb_sub64;
    logic [RISCV_ARCH-1:0] vb_memop_memaddr;
    logic [RISCV_ARCH-1:0] vb_memop_memaddr_load;
    logic [RISCV_ARCH-1:0] vb_memop_memaddr_store;
    logic [RISCV_ARCH-1:0] vb_memop_wdata;
    logic [Instr_Total-1:0] wv;
    logic [2:0] opcode_len;
    logic v_call;
    logic v_ret;
    logic v_pc_branch;
    logic v_eq;                                             // equal
    logic v_ge;                                             // greater/equal signed
    logic v_geu;                                            // greater/equal unsigned
    logic v_lt;                                             // less signed
    logic v_ltu;                                            // less unsigned
    logic v_neq;                                            // not equal
    logic [RISCV_ARCH-1:0] vb_rdata1;
    logic [RISCV_ARCH-1:0] vb_rdata2;
    logic [RISCV_ARCH-1:0] vb_rdata1_amo;
    logic v_check_tag1;
    logic v_check_tag2;
    logic [Res_Total-1:0] vb_select;
    logic [(CFG_REG_TAG_WIDTH * REGS_TOTAL)-1:0] vb_tagcnt_next;
    logic v_latch_input;
    logic v_memop_ena;
    logic v_memop_debug;
    logic v_reg_ena;
    logic [5:0] vb_reg_waddr;
    logic v_instr_misaligned;
    logic v_store_misaligned;
    logic v_load_misaligned;
    logic v_debug_misaligned;                               // from the debug interface
    logic v_csr_cmd_ena;
    logic v_mem_ex;
    logic [11:0] vb_csr_cmd_addr;
    logic [CsrReq_TotalBits-1:0] vb_csr_cmd_type;
    logic v_dbg_mem_req_ready;
    logic v_dbg_mem_req_error;
    logic v_halted;
    logic v_idle;
    input_mux_type mux;
    logic [RISCV_ARCH-1:0] vb_o_npc;
    int t_radr1;
    int t_radr2;
    int t_waddr;
    logic [MemopType_Total-1:0] t_type;
    logic [CFG_REG_TAG_WIDTH-1:0] t_tagcnt_wr;
    logic [2:0] t_alu_mode;
    logic [6:0] t_addsub_mode;
    logic [3:0] t_shifter_mode;

    v_d_valid = 0;
    v_csr_req_valid = 0;
    v_csr_resp_ready = 0;
    vb_csr_cmd_wdata = 0;
    vb_res = 0;
    vb_prog_npc = 0;
    vb_npc_incr = 0;
    vb_off = 0;
    vb_sub64 = 0;
    vb_memop_memaddr = 0;
    vb_memop_memaddr_load = 0;
    vb_memop_memaddr_store = 0;
    vb_memop_wdata = 0;
    wv = 0;
    opcode_len = 0;
    v_call = 0;
    v_ret = 0;
    v_pc_branch = 0;
    v_eq = 1'h0;
    v_ge = 1'h0;
    v_geu = 1'h0;
    v_lt = 1'h0;
    v_ltu = 1'h0;
    v_neq = 1'h0;
    vb_rdata1 = 0;
    vb_rdata2 = 0;
    vb_rdata1_amo = 0;
    v_check_tag1 = 0;
    v_check_tag2 = 0;
    vb_select = 0;
    vb_tagcnt_next = 0;
    v_latch_input = 0;
    v_memop_ena = 0;
    v_memop_debug = 0;
    v_reg_ena = 0;
    vb_reg_waddr = 0;
    v_instr_misaligned = 0;
    v_store_misaligned = 0;
    v_load_misaligned = 0;
    v_debug_misaligned = 1'h0;
    v_csr_cmd_ena = 0;
    v_mem_ex = 0;
    vb_csr_cmd_addr = 0;
    vb_csr_cmd_type = 0;
    v_dbg_mem_req_ready = 0;
    v_dbg_mem_req_error = 0;
    v_halted = 0;
    v_idle = 0;
    vb_o_npc = 0;
    t_radr1 = 0;
    t_radr2 = 0;
    t_waddr = 0;
    t_type = 0;
    t_tagcnt_wr = 0;
    t_alu_mode = 0;
    t_addsub_mode = 0;
    t_shifter_mode = 0;

    v = r;

    v.valid = 1'b0;
    v.call = 1'b0;
    v.ret = 1'b0;
    v.reg_write = 1'b0;
    for (int i = 0; i < Res_Total; i++) begin
        wb_select[i].ena = '0;
    end
    vb_reg_waddr = i_d_waddr;

    if (r.state == State_Idle) begin
        v_idle = 1'b1;
        mux.radr1 = i_d_radr1;
        mux.radr2 = i_d_radr2;
        mux.waddr = i_d_waddr;
        mux.imm = i_d_imm;
        mux.pc = i_d_pc;
        mux.instr = i_d_instr;
        mux.memop_type[MemopType_Store] = i_memop_store;
        mux.memop_type[MemopType_Locked] = (i_amo & i_memop_load);
        mux.memop_type[MemopType_Reserve] = (i_ivec[Instr_LR_D] || i_ivec[Instr_LR_W]);
        mux.memop_type[MemopType_Release] = (i_ivec[Instr_SC_D] || i_ivec[Instr_SC_W]);
        mux.memop_sign_ext = i_memop_sign_ext;
        mux.memop_size = i_memop_size;
        mux.unsigned_op = i_unsigned_op;
        mux.rv32 = i_rv32;
        mux.compressed = i_compressed;
        mux.f64 = i_f64;
        mux.ivec = i_ivec;
        mux.isa_type = i_isa_type;
    end else begin
        mux.radr1 = r.radr1;
        mux.radr2 = r.radr2;
        mux.waddr = r.waddr;
        mux.imm = r.imm;
        mux.pc = r.pc;
        mux.instr = r.instr;
        mux.memop_type = r.memop_type;
        mux.memop_sign_ext = r.memop_sign_ext;
        mux.memop_size = r.memop_size;
        mux.unsigned_op = r.unsigned_op;
        mux.rv32 = r.rv32;
        mux.compressed = r.compressed;
        mux.f64 = r.f64;
        mux.ivec = r.ivec;
        mux.isa_type = r.isa_type;
    end
    wv = mux.ivec;

    if (r.state == State_Amo) begin
        // AMO R-type:
        vb_rdata1 = r.rdata1_amo;
        vb_rdata2 = r.rdata2_amo;
        v_check_tag1 = 1'b1;
        v_check_tag2 = 1'b1;
    end else if (mux.isa_type[ISA_R_type] == 1'b1) begin
        vb_rdata1 = i_rdata1;
        vb_rdata2 = i_rdata2;
        v_check_tag1 = 1'b1;
        v_check_tag2 = 1'b1;
    end else if (mux.isa_type[ISA_I_type] == 1'b1) begin
        vb_rdata1 = i_rdata1;
        vb_rdata2 = mux.imm;
        v_check_tag1 = 1'b1;
    end else if (mux.isa_type[ISA_SB_type] == 1'b1) begin
        vb_rdata1 = i_rdata1;
        vb_rdata2 = i_rdata2;
        vb_off = mux.imm;
        v_check_tag1 = 1'b1;
        v_check_tag2 = 1'b1;
    end else if (mux.isa_type[ISA_UJ_type] == 1'b1) begin
        vb_rdata1 = mux.pc;
        vb_off = mux.imm;
        v_check_tag1 = 1'b1;
    end else if (mux.isa_type[ISA_U_type] == 1'b1) begin
        vb_rdata1 = mux.pc;
        vb_rdata2 = mux.imm;
    end else if (mux.isa_type[ISA_S_type] == 1'b1) begin
        vb_rdata1 = i_rdata1;
        vb_rdata2 = i_rdata2;
        vb_off = mux.imm;
        v_check_tag1 = 1'b1;
        v_check_tag2 = 1'b1;
    end
    // AMO value read from memory[rs1]
    if ((wv[Instr_AMOSWAP_D] || wv[Instr_AMOSWAP_W]) == 1'b1) begin
        vb_rdata1_amo = '0;
    end else if (i_mem_valid == 1'b1) begin
        if (mux.rv32 == 1'b1) begin
            // All AMO are sign-extended:
            if (r.memop_memaddr[2] == 1'b1) begin
                vb_rdata1_amo[31: 0] = i_mem_rdata[63: 32];
                if ((mux.memop_sign_ext == 1'b1) && (i_mem_rdata[63] == 1'b1)) begin
                    vb_rdata1_amo[63: 32] = '1;
                end else begin
                    vb_rdata1_amo[63: 32] = '0;
                end
            end else begin
                vb_rdata1_amo[31: 0] = i_mem_rdata[31: 0];
                if ((mux.memop_sign_ext == 1'b1) && (i_mem_rdata[31] == 1'b1)) begin
                    vb_rdata1_amo[63: 32] = '1;
                end else begin
                    vb_rdata1_amo[63: 32] = '0;
                end
            end
        end else begin
            vb_rdata1_amo = i_mem_rdata;
        end
    end
    v.rdata1_amo = vb_rdata1_amo;

    vb_memop_memaddr_load = (vb_rdata1 + vb_rdata2);
    vb_memop_memaddr_store = (vb_rdata1 + vb_off);
    if (mux.memop_type[MemopType_Store] == 1'b0) begin
        vb_memop_memaddr = vb_memop_memaddr_load;
    end else begin
        vb_memop_memaddr = vb_memop_memaddr_store;
    end

    // Check that registers tags are equal to expeted ones
    t_radr1 = int'(mux.radr1);
    t_radr2 = int'(mux.radr2);
    w_hazard1 = 1'b0;
    if (r.tagcnt[(CFG_REG_TAG_WIDTH * t_radr1) +: CFG_REG_TAG_WIDTH] != i_rtag1) begin
        w_hazard1 = v_check_tag1;
    end
    w_hazard2 = 1'b0;
    if (r.tagcnt[(CFG_REG_TAG_WIDTH * t_radr2) +: CFG_REG_TAG_WIDTH] != i_rtag2) begin
        w_hazard2 = v_check_tag2;
    end

    // Compute branch conditions:
    vb_sub64 = (vb_rdata1 - vb_rdata2);
    v_eq = (~(|vb_sub64));                                  // equal
    v_ge = (~vb_sub64[63]);                                 // greater/equal (signed)
    v_geu = 1'b0;
    if (vb_rdata1 >= vb_rdata2) begin
        v_geu = 1'b1;                                       // greater/equal (unsigned)
    end
    v_lt = vb_sub64[63];                                    // less (signed)
    v_ltu = 1'b0;
    if (vb_rdata1 < vb_rdata2) begin
        v_ltu = 1'b1;                                       // less (unsiged)
    end
    v_neq = (|vb_sub64);                                    // not equal

    // Relative Branch on some condition:
    v_pc_branch = 1'b0;
    if (((wv[Instr_BEQ] && v_eq)
            || (wv[Instr_BGE] && v_ge)
            || (wv[Instr_BGEU] && v_geu)
            || (wv[Instr_BLT] && v_lt)
            || (wv[Instr_BLTU] && v_ltu)
            || (wv[Instr_BNE] && v_neq)) == 1'b1) begin
        v_pc_branch = 1'b1;
    end

    wb_fpu_vec = wv[Instr_FPU_Last: Instr_FPU_First];       // directly connected i_ivec
    w_arith_residual_high = (wv[Instr_REM]
            || wv[Instr_REMU]
            || wv[Instr_REMW]
            || wv[Instr_REMUW]
            || wv[Instr_MULH]
            || wv[Instr_MULHSU]
            || wv[Instr_MULHU]);
    w_mul_hsu = wv[Instr_MULHSU];

    v_instr_misaligned = mux.pc[0];
    if (((wv[Instr_LD] == 1'b1) && ((|vb_memop_memaddr_load[2: 0]) == 1'b1))
            || (((wv[Instr_LW] || wv[Instr_LWU]) == 1'b1) && ((|vb_memop_memaddr_load[1: 0]) == 1'b1))
            || (((wv[Instr_LH] || wv[Instr_LHU]) == 1'b1) && (vb_memop_memaddr_load[0] == 1'b1))) begin
        v_load_misaligned = 1'b1;
    end
    if (((wv[Instr_SD] && ((|vb_memop_memaddr_store[2: 0]) == 1'b1)) == 1'b1)
            || ((wv[Instr_SW] && ((|vb_memop_memaddr_store[1: 0]) == 1'b1)) == 1'b1)
            || ((wv[Instr_SH] && (vb_memop_memaddr_store[0] == 1'b1)) == 1'b1)) begin
        v_store_misaligned = 1'b1;
    end
    if (((i_dbg_mem_req_size == 2'h3) && ((|i_dbg_mem_req_addr[2: 0]) == 1'b1))
            || ((i_dbg_mem_req_size == 2'h2) && ((|i_dbg_mem_req_addr[1: 0]) == 1'b1))
            || ((i_dbg_mem_req_size == 2'h1) && (i_dbg_mem_req_addr[0] == 1'b1))) begin
        v_debug_misaligned = 1'b1;
    end
    if (i_stack_overflow == 1'b1) begin
        v.stack_overflow = 1'b1;
    end
    if (i_stack_underflow == 1'b1) begin
        v.stack_underflow = 1'b1;
    end
    if ((i_mem_ex_load_fault == 1'b1) && (i_mem_ex_debug == 1'b0)) begin
        v.mem_ex_load_fault = 1'b1;
        v.mem_ex_addr = i_mem_ex_addr;
    end
    if ((i_mem_ex_store_fault == 1'b1) && (i_mem_ex_debug == 1'b0)) begin
        v.mem_ex_store_fault = 1'b1;
        v.mem_ex_addr = i_mem_ex_addr;
    end
    if ((i_page_fault_r == 1'b1) && (i_mem_ex_debug == 1'b0)) begin
        v.page_fault_r = 1'b1;
        v.mem_ex_addr = i_mem_ex_addr;
    end
    if ((i_page_fault_w == 1'b1) && (i_mem_ex_debug == 1'b0)) begin
        v.page_fault_w = 1'b1;
        v.mem_ex_addr = i_mem_ex_addr;
    end

    opcode_len = 3'h4;
    if (mux.compressed == 1'b1) begin
        opcode_len = 3'h2;
    end
    vb_npc_incr = (mux.pc + opcode_len);

    if (v_pc_branch == 1'b1) begin
        vb_prog_npc = (mux.pc + vb_off);
    end else if (wv[Instr_JAL] == 1'b1) begin
        vb_prog_npc = (vb_rdata1 + vb_off);
    end else if (wv[Instr_JALR] == 1'b1) begin
        vb_prog_npc = (vb_rdata1 + vb_rdata2);
        vb_prog_npc[0] = 1'b0;
    end else begin
        vb_prog_npc = vb_npc_incr;
    end

    vb_select[Res_Reg2] = (mux.memop_type[MemopType_Store]
            || wv[Instr_LUI]);
    vb_select[Res_Npc] = 1'b0;
    vb_select[Res_Ra] = (v_pc_branch
            || wv[Instr_JAL]
            || wv[Instr_JALR]
            || wv[Instr_MRET]
            || wv[Instr_HRET]
            || wv[Instr_SRET]
            || wv[Instr_URET]);
    vb_select[Res_Csr] = (wv[Instr_CSRRC]
            || wv[Instr_CSRRCI]
            || wv[Instr_CSRRS]
            || wv[Instr_CSRRSI]
            || wv[Instr_CSRRW]
            || wv[Instr_CSRRWI]);
    vb_select[Res_Alu] = (wv[Instr_AND]
            || wv[Instr_ANDI]
            || wv[Instr_OR]
            || wv[Instr_ORI]
            || wv[Instr_XOR]
            || wv[Instr_XORI]
            || wv[Instr_AMOOR_D]
            || wv[Instr_AMOOR_W]
            || wv[Instr_AMOAND_D]
            || wv[Instr_AMOAND_W]
            || wv[Instr_AMOXOR_D]
            || wv[Instr_AMOXOR_W]);
    vb_select[Res_AddSub] = (wv[Instr_ADD]
            || wv[Instr_ADDI]
            || wv[Instr_AUIPC]
            || wv[Instr_ADDW]
            || wv[Instr_ADDIW]
            || wv[Instr_SUB]
            || wv[Instr_SUBW]
            || wv[Instr_SLT]
            || wv[Instr_SLTI]
            || wv[Instr_SLTU]
            || wv[Instr_SLTIU]
            || wv[Instr_AMOADD_D]
            || wv[Instr_AMOADD_W]
            || wv[Instr_AMOMIN_D]
            || wv[Instr_AMOMIN_W]
            || wv[Instr_AMOMAX_D]
            || wv[Instr_AMOMAX_W]
            || wv[Instr_AMOMINU_D]
            || wv[Instr_AMOMINU_W]
            || wv[Instr_AMOMAXU_D]
            || wv[Instr_AMOMAXU_W]
            || wv[Instr_AMOSWAP_D]
            || wv[Instr_AMOSWAP_W]);
    vb_select[Res_Shifter] = (wv[Instr_SLL]
            || wv[Instr_SLLI]
            || wv[Instr_SLLW]
            || wv[Instr_SLLIW]
            || wv[Instr_SRL]
            || wv[Instr_SRLI]
            || wv[Instr_SRLW]
            || wv[Instr_SRLIW]
            || wv[Instr_SRA]
            || wv[Instr_SRAI]
            || wv[Instr_SRAW]
            || wv[Instr_SRAW]
            || wv[Instr_SRAIW]);
    vb_select[Res_IMul] = (wv[Instr_MUL]
            || wv[Instr_MULW]
            || wv[Instr_MULH]
            || wv[Instr_MULHSU]
            || wv[Instr_MULHU]);
    vb_select[Res_IDiv] = (wv[Instr_DIV]
            || wv[Instr_DIVU]
            || wv[Instr_DIVW]
            || wv[Instr_DIVUW]
            || wv[Instr_REM]
            || wv[Instr_REMU]
            || wv[Instr_REMW]
            || wv[Instr_REMUW]);
    if (fpu_ena) begin
        vb_select[Res_FPU] = (mux.f64 && (~(wv[Instr_FSD] || wv[Instr_FLD])));
    end
    vb_select[Res_Zero] = (~(|vb_select[(Res_Total - 1): (Res_Zero + 1)]));// load memory, fence

    if (((wv[Instr_JAL] || wv[Instr_JALR]) == 1'b1) && (mux.waddr == REG_RA)) begin
        v_call = 1'b1;
    end
    if ((wv[Instr_JALR] == 1'b1)
            && ((|vb_rdata2) == 1'b0)
            && (mux.waddr != REG_RA)
            && (mux.radr1 == REG_RA)) begin
        v_ret = 1'b1;
    end

    v_mem_ex = (r.mem_ex_load_fault
            || r.mem_ex_store_fault
            || i_page_fault_x
            || r.page_fault_r
            || r.page_fault_w);
    v_csr_cmd_ena = (i_haltreq
            || (i_step && r.stepdone)
            || i_unsup_exception
            || i_instr_load_fault
            || v_mem_ex
            || r.stack_overflow
            || r.stack_underflow
            || v_instr_misaligned
            || v_load_misaligned
            || v_store_misaligned
            || (|i_irq_pending)
            || wv[Instr_WFI]
            || wv[Instr_EBREAK]
            || wv[Instr_ECALL]
            || wv[Instr_MRET]
            || wv[Instr_HRET]
            || wv[Instr_SRET]
            || wv[Instr_URET]
            || wv[Instr_CSRRC]
            || wv[Instr_CSRRCI]
            || wv[Instr_CSRRS]
            || wv[Instr_CSRRSI]
            || wv[Instr_CSRRW]
            || wv[Instr_CSRRWI]
            || wv[Instr_FENCE]
            || wv[Instr_FENCE_I]
            || wv[Instr_SFENCE_VMA]);
    if (wv[Instr_CSRRC] == 1'b1) begin
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata = (i_csr_resp_data & (~r.rdata1));
    end else if (wv[Instr_CSRRCI] == 1'b1) begin
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata[(RISCV_ARCH - 1): 5] = i_csr_resp_data[(RISCV_ARCH - 1): 5];
        vb_csr_cmd_wdata[4: 0] = (i_csr_resp_data[4: 0] & (~r.radr1[4: 0]));// zero-extending 5 to 64-bits
    end else if (wv[Instr_CSRRS] == 1'b1) begin
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata = (i_csr_resp_data | r.rdata1);
    end else if (wv[Instr_CSRRSI] == 1'b1) begin
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata[(RISCV_ARCH - 1): 5] = i_csr_resp_data[(RISCV_ARCH - 1): 5];
        vb_csr_cmd_wdata[4: 0] = (i_csr_resp_data[4: 0] | r.radr1[4: 0]);// zero-extending 5 to 64-bits
    end else if (wv[Instr_CSRRW] == 1'b1) begin
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata = r.rdata1;
    end else if (wv[Instr_CSRRWI] == 1'b1) begin
        vb_csr_cmd_type = CsrReq_ReadCmd;
        vb_csr_cmd_addr = i_d_csr_addr;
        vb_csr_cmd_wdata[4: 0] = r.radr1[4: 0];             // zero-extending 5 to 64-bits
    end
    // Higher priority CSR requests. They can redefine CSR command only in the Idle state,
    // otherwise it is possible situation when read-modify-write sequence uses wrong cmd_wdata
    if (v_idle) begin
        if (i_haltreq == 1'b1) begin
            vb_csr_cmd_type = CsrReq_HaltCmd;
            vb_csr_cmd_addr = HALT_CAUSE_HALTREQ;
        end else if ((i_step == 1'b1) && (r.stepdone == 1'b1)) begin
            vb_csr_cmd_type = CsrReq_HaltCmd;
            vb_csr_cmd_addr = HALT_CAUSE_STEP;
        end else if (v_instr_misaligned == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_InstrMisalign;      // Instruction address misaligned
            vb_csr_cmd_wdata = mux.pc;
        end else if (i_instr_load_fault == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_InstrFault;         // Instruction access fault
            vb_csr_cmd_wdata = mux.pc;
        end else if (i_unsup_exception) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_InstrIllegal;       // Illegal instruction
            vb_csr_cmd_wdata = mux.instr;
        end else if (wv[Instr_EBREAK] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_BreakpointCmd;
            vb_csr_cmd_addr = EXCEPTION_Breakpoint;
        end else if (v_load_misaligned == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_LoadMisalign;       // Load address misaligned
            vb_csr_cmd_wdata = vb_memop_memaddr_load;
        end else if (r.mem_ex_load_fault == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_LoadFault;          // Load access fault
            vb_csr_cmd_wdata = r.mem_ex_addr;
        end else if (v_store_misaligned == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_StoreMisalign;      // Store/AMO address misaligned
            vb_csr_cmd_wdata = vb_memop_memaddr_store;
        end else if (r.mem_ex_store_fault == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_StoreFault;         // Store/AMO access fault
            vb_csr_cmd_wdata = r.mem_ex_addr;
        end else if (i_page_fault_x == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_InstrPageFault;     // Instruction fetch page fault
            vb_csr_cmd_wdata = mux.pc;
        end else if (r.page_fault_r == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_LoadPageFault;      // Data load page fault
            vb_csr_cmd_wdata = r.mem_ex_addr;
        end else if (r.page_fault_w == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_StorePageFault;     // Data store page fault
            vb_csr_cmd_wdata = r.mem_ex_addr;
        end else if (r.stack_overflow == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_StackOverflow;      // Stack overflow
        end else if (r.stack_underflow == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_StackUnderflow;     // Stack Underflow
        end else if (wv[Instr_ECALL] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_ExceptionCmd;
            vb_csr_cmd_addr = EXCEPTION_CallFromXMode;      // Environment call
        end else if ((|i_irq_pending) == 1'b1) begin
            vb_csr_cmd_type = CsrReq_InterruptCmd;
            vb_csr_cmd_addr = irq2idx(i_irq_pending);
        end else if (wv[Instr_WFI] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_WfiCmd;
            vb_csr_cmd_addr = mux.instr[14: 12];            // PRIV field
        end else if (wv[Instr_MRET] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_TrapReturnCmd;
            vb_csr_cmd_addr = PRV_M;
        end else if (wv[Instr_HRET] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_TrapReturnCmd;
            vb_csr_cmd_addr = PRV_H;
        end else if (wv[Instr_SRET] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_TrapReturnCmd;
            vb_csr_cmd_addr = PRV_S;
        end else if (wv[Instr_URET] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_TrapReturnCmd;
            vb_csr_cmd_addr = PRV_U;
        end else if (wv[Instr_FENCE] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_FenceCmd;
            vb_csr_cmd_addr = 12'h001;                      // [0]=fence; [1] fence_i [2]=vma
        end else if (wv[Instr_FENCE_I] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_FenceCmd;
            vb_csr_cmd_addr = 12'h002;                      // [0]=fence; [1] fence_i [2]=vma
            vb_csr_cmd_wdata = '1;                          // flush address
        end else if (wv[Instr_SFENCE_VMA] == 1'b1) begin
            vb_csr_cmd_type = CsrReq_FenceCmd;
            vb_csr_cmd_addr = 12'h004;                      // [0]=fence; [1] fence_i [2]=vma
            if ((|mux.radr1) == 1'b0) begin                 // must be set to zero in standard extension for fence and fence.i 
                vb_csr_cmd_wdata = '1;                      // flush address
            end else begin
                vb_csr_cmd_wdata = i_rdata1;                // flush specific address
            end
            // rs2 register contains Adress Space ID (asid) or Guest Space ID (gsid). Only one MMU implemented.
        end
    end

    wb_select[Res_Zero].res = '0;
    wb_select[Res_Reg2].res = r.rdata2;
    wb_select[Res_Csr].res = r.res_csr;
    wb_select[Res_Npc].res = r.res_npc;
    wb_select[Res_Ra].res = r.res_ra;

    // Select result:
    if (r.select[Res_Reg2] == 1'b1) begin
        vb_res = wb_select[Res_Reg2].res;
    end else if (r.select[Res_Npc] == 1'b1) begin
        vb_res = wb_select[Res_Npc].res;
    end else if (r.select[Res_Ra] == 1'b1) begin
        vb_res = wb_select[Res_Ra].res;
    end else if (r.select[Res_Csr] == 1'b1) begin
        vb_res = wb_select[Res_Csr].res;
    end else if (r.select[Res_Alu] == 1'b1) begin
        vb_res = wb_select[Res_Alu].res;
    end else if (r.select[Res_AddSub] == 1'b1) begin
        vb_res = wb_select[Res_AddSub].res;
    end else if (r.select[Res_Shifter] == 1'b1) begin
        vb_res = wb_select[Res_Shifter].res;
    end else if (r.select[Res_IMul] == 1'b1) begin
        vb_res = wb_select[Res_IMul].res;
    end else if (r.select[Res_IDiv] == 1'b1) begin
        vb_res = wb_select[Res_IDiv].res;
    end else if (r.select[Res_FPU] == 1'b1) begin
        vb_res = wb_select[Res_FPU].res;
    end else begin
        vb_res = '0;
    end

    if (((i_d_pc == r.npc)
                    && (i_d_progbuf_ena == 1'b0)
                    && (i_dbg_progbuf_ena == 1'b0))
            || ((i_d_pc == r.dnpc)
                    && (i_d_progbuf_ena == 1'b1)
                    && (i_dbg_progbuf_ena == 1'b1))) begin
        v_d_valid = 1'b1;
    end

    case (r.state)
    State_Idle: begin
        if ((r.memop_valid == 1'b1) && (i_memop_ready == 1'b0)) begin
            // Do nothing, previous memaccess request wasn't accepted. queue is full.
        end else if ((v_d_valid == 1'b1) && (w_hazard1 == 1'b0) && (w_hazard2 == 1'b0)) begin
            v_latch_input = 1'b1;
            // opencocd doesn't clear 'step' value in dcsr after step has been done
            v.stepdone = (i_step && (~i_dbg_progbuf_ena));
            if (i_dbg_mem_req_valid == 1'b1) begin
                v_dbg_mem_req_ready = 1'b1;
                v_dbg_mem_req_error = v_debug_misaligned;
                if (v_debug_misaligned == 1'b1) begin
                    v.state = State_DebugMemError;
                end else begin
                    v.state = State_DebugMemRequest;
                end
                v.memop_halted = 1'b0;
                v.memop_sign_ext = 1'b0;
                t_type[MemopType_Store] = i_dbg_mem_req_write;
                v.memop_type = t_type;
                v.memop_size = i_dbg_mem_req_size;
            end else if (v_csr_cmd_ena == 1'b1) begin
                v.state = State_Csr;
                v.csrstate = CsrState_Req;
                v.csr_req_type = vb_csr_cmd_type;
                v.csr_req_addr = vb_csr_cmd_addr;
                v.csr_req_data = vb_csr_cmd_wdata;
                v.csr_req_rmw = vb_csr_cmd_type[CsrReq_ReadBit];// read/modify/write
                v.mem_ex_load_fault = 1'b0;
                v.mem_ex_store_fault = 1'b0;
                v.page_fault_r = 1'b0;
                v.page_fault_w = 1'b0;
                v.stack_overflow = 1'b0;
                v.stack_underflow = 1'b0;
            end else if ((vb_select[Res_IMul] || vb_select[Res_IDiv] || vb_select[Res_FPU]) == 1'b1) begin
                v.state = State_WaitMulti;
            end else if (i_amo == 1'b1) begin
                v_memop_ena = 1'b1;
                vb_memop_memaddr = vb_rdata1;
                vb_memop_wdata = vb_rdata2;
                v.state = State_Amo;
                if (i_memop_ready == 1'b0) begin
                    v.amostate = AmoState_WaitMemAccess;
                end else begin
                    v.amostate = AmoState_Read;
                end
            end else if ((i_memop_load || i_memop_store) == 1'b1) begin
                v_memop_ena = 1'b1;
                vb_memop_wdata = vb_rdata2;
                if (i_memop_ready == 1'b0) begin
                    // Wait cycles until FIFO to memoryaccess becomes available
                    v.state = State_WaitMemAcces;
                end else begin
                    v.valid = 1'b1;
                end
            end else begin
                v.valid = 1'b1;
                v_reg_ena = ((|i_d_waddr) && (~i_memop_load));// should be written by memaccess, but tag must be updated
            end
        end
    end
    State_WaitMemAcces: begin
        // Fifo exec => memacess is full
        vb_memop_memaddr = r.memop_memaddr;
        if (i_memop_ready == 1'b1) begin
            v.state = State_Idle;
            v.valid = 1'b1;
        end
    end
    State_Csr: begin
        // Request throught CSR bus
        case (r.csrstate)
        CsrState_Req: begin
            v_csr_req_valid = 1'b1;
            if (i_csr_req_ready == 1'b1) begin
                v.csrstate = CsrState_Resp;
            end
        end
        CsrState_Resp: begin
            v_csr_resp_ready = 1'b1;
            if (i_csr_resp_valid == 1'b1) begin
                v.csrstate = CsrState_Idle;
                if (i_csr_resp_exception == 1'b1) begin
                    if (i_dbg_progbuf_ena == 1'b1) begin
                        v.valid = 1'b0;
                        v.state = State_Halted;
                    end else begin
                        // Invalid access rights
                        v.csrstate = CsrState_Req;
                        v.csr_req_type = CsrReq_ExceptionCmd;
                        v.csr_req_addr = EXCEPTION_InstrIllegal;
                        v.csr_req_data = mux.instr;
                        v.csr_req_rmw = 1'b0;
                    end
                end else if (r.csr_req_type[CsrReq_HaltBit] == 1'b1) begin
                    v.valid = 1'b0;
                    v.state = State_Halted;
                end else if (r.csr_req_type[CsrReq_BreakpointBit] == 1'b1) begin
                    v.valid = 1'b0;
                    if (i_csr_resp_data[0] == 1'b1) begin
                        // ebreakm is set
                        v.state = State_Halted;
                    end else begin
                        v.state = State_Idle;
                        if (i_dbg_progbuf_ena == 1'b0) begin
                            v.npc = i_csr_resp_data;
                        end
                    end
                end else if ((r.csr_req_type[CsrReq_ExceptionBit]
                            || r.csr_req_type[CsrReq_InterruptBit]
                            || r.csr_req_type[CsrReq_ResumeBit]) == 1'b1) begin
                    v.valid = wv[Instr_ECALL];              // No valid strob should be generated for all exceptions except ECALL
                    v.state = State_Idle;
                    if (i_dbg_progbuf_ena == 1'b0) begin
                        v.npc = i_csr_resp_data;
                    end
                end else if (r.csr_req_type[CsrReq_WfiBit] == 1'b1) begin
                    if (i_csr_resp_data[0] == 1'b1) begin
                        // Invalid WFI instruction in current mode
                        v.csrstate = CsrState_Req;
                        v.csr_req_type = CsrReq_ExceptionCmd;
                        v.csr_req_addr = EXCEPTION_InstrIllegal;
                        v.csr_req_data = mux.instr;
                        v.csr_req_rmw = 1'b0;
                    end else begin
                        v.valid = 1'b1;
                        v.state = State_Wfi;
                    end
                end else if (r.csr_req_type[CsrReq_TrapReturnBit] == 1'b1) begin
                    v.valid = 1'b1;
                    v.state = State_Idle;
                    if (i_dbg_progbuf_ena == 1'b0) begin
                        v.npc = i_csr_resp_data;
                    end
                end else if (r.csr_req_rmw == 1'b1) begin
                    v.csrstate = CsrState_Req;
                    v.csr_req_type = CsrReq_WriteCmd;
                    v.csr_req_data = vb_csr_cmd_wdata;
                    v.csr_req_rmw = 1'b0;

                    // Store result int cpu register on next clock
                    v.res_csr = i_csr_resp_data;
                    v_reg_ena = (|r.waddr);
                    vb_reg_waddr = r.waddr;
                end else begin
                    v.state = State_Idle;
                    v.valid = 1'b1;
                end
            end
        end
        default: begin
        end
        endcase
    end
    State_Amo: begin
        case (r.amostate)
        AmoState_WaitMemAccess: begin
            // No need to make memop_valid active
            if (i_memop_ready == 1'b1) begin
                v.amostate = AmoState_Read;
            end
        end
        AmoState_Read: begin
            v.rdata2_amo = i_rdata2;
            v.amostate = AmoState_Modify;
        end
        AmoState_Modify: begin
            if (i_memop_idle == 1'b1) begin
                // Need to wait 1 clock to latch addsub/alu output
                v.amostate = AmoState_Write;
                mux.memop_type[MemopType_Store] = 1'b1;
                v.memop_type = mux.memop_type;
            end
        end
        AmoState_Write: begin
            v_memop_ena = 1'b1;
            vb_memop_memaddr = r.memop_memaddr;
            vb_memop_wdata = vb_res;
            if (i_memop_ready == 1'b1) begin
                v.state = State_Idle;
                v.amostate = AmoState_WaitMemAccess;
                v.valid = 1'b1;
            end
        end
        default: begin
        end
        endcase
    end
    State_WaitMulti: begin
        // Wait end of multiclock instructions
        if ((wb_select[Res_IMul].valid
                || wb_select[Res_IDiv].valid
                || wb_select[Res_FPU].valid) == 1'b1) begin
            v.state = State_Idle;
            v_reg_ena = (|r.waddr);
            vb_reg_waddr = r.waddr;
            v.valid = 1'b1;
        end
    end
    State_Halted: begin
        v.stepdone = 1'b0;
        if ((i_resumereq == 1'b1) || (i_dbg_progbuf_ena == 1'b1)) begin
            v.state = State_Csr;
            v.csrstate = CsrState_Req;
            v.csr_req_type = CsrReq_ResumeCmd;
            v.csr_req_addr = '0;
            v.csr_req_data = '0;
        end else if (i_dbg_mem_req_valid == 1'b1) begin
            v_dbg_mem_req_ready = 1'b1;
            v_dbg_mem_req_error = v_debug_misaligned;
            if (v_debug_misaligned == 1'b1) begin
                v.state = State_DebugMemError;
            end else begin
                v.state = State_DebugMemRequest;
            end
            v.memop_halted = 1'b1;
            v.memop_sign_ext = 1'b0;
            t_type[MemopType_Store] = i_dbg_mem_req_write;
            v.memop_type = t_type;
            v.memop_size = i_dbg_mem_req_size;
        end
    end
    State_DebugMemRequest: begin
        v_memop_ena = 1'b1;
        v_memop_debug = 1'b1;
        vb_memop_memaddr = i_dbg_mem_req_addr;
        vb_memop_wdata = i_dbg_mem_req_wdata;
        if (i_memop_ready == 1'b1) begin
            if (r.memop_halted == 1'b1) begin
                v.state = State_Halted;
            end else begin
                v.state = State_Idle;
            end
        end
    end
    State_DebugMemError: begin
        if (r.memop_halted == 1'b1) begin
            v.state = State_Halted;
        end else begin
            v.state = State_Idle;
        end
    end
    State_Wfi: begin
        if ((i_haltreq || i_wakeup) == 1'b1) begin
            v.state = State_Idle;
        end
    end
    default: begin
    end
    endcase

    // Next tags:
    t_waddr = int'(vb_reg_waddr);

    t_tagcnt_wr = (r.tagcnt[(CFG_REG_TAG_WIDTH * t_waddr) +: CFG_REG_TAG_WIDTH] + 1);

    vb_tagcnt_next = r.tagcnt;
    vb_tagcnt_next[(CFG_REG_TAG_WIDTH * t_waddr) +: CFG_REG_TAG_WIDTH] = t_tagcnt_wr;
    vb_tagcnt_next[(CFG_REG_TAG_WIDTH - 1): 0] = '0;        // r0 always 0
    if (i_dbg_progbuf_ena == 1'b0) begin
        v.dnpc = '0;
    end

    // Latch decoder's data into internal registers:
    if (v_latch_input == 1'b1) begin
        if (i_dbg_progbuf_ena == 1'b1) begin
            v.dnpc = (r.dnpc + opcode_len);
        end else begin
            v.dnpc = '0;
            v.pc = i_d_pc;
            v.npc = vb_prog_npc;                            // Actually this value will be restored on resume request
        end
        v.radr1 = i_d_radr1;
        v.radr2 = i_d_radr2;
        v.waddr = i_d_waddr;
        v.rdata1 = vb_rdata1;
        v.rdata2 = vb_rdata2;
        v.imm = i_d_imm;
        v.ivec = i_ivec;
        v.isa_type = i_isa_type;
        v.unsigned_op = i_unsigned_op;
        v.rv32 = i_rv32;
        v.compressed = i_compressed;
        v.f64 = i_f64;
        v.instr = i_d_instr;
        v.call = v_call;
        v.ret = v_ret;
        v.jmp = (v_pc_branch
                || wv[Instr_JAL]
                || wv[Instr_JALR]
                || wv[Instr_MRET]
                || wv[Instr_HRET]
                || wv[Instr_SRET]
                || wv[Instr_URET]);
        v.res_npc = vb_prog_npc;
        v.res_ra = vb_npc_incr;

        wb_select[Res_IMul].ena = vb_select[Res_IMul];
        wb_select[Res_IDiv].ena = vb_select[Res_IDiv];
        wb_select[Res_FPU].ena = vb_select[Res_FPU];
        v.select = vb_select;
    end
    if (v_reg_ena == 1'b1) begin
        v.reg_write = 1'b1;
        v.tagcnt = vb_tagcnt_next;
        v.reg_waddr = vb_reg_waddr;
        v.reg_wtag = t_tagcnt_wr;
    end
    if (v_memop_ena == 1'b1) begin
        v.memop_valid = 1'b1;
        v.memop_debug = v_memop_debug;
        v.memop_type = mux.memop_type;
        v.memop_sign_ext = mux.memop_sign_ext;
        v.memop_size = mux.memop_size;
        v.memop_memaddr = vb_memop_memaddr;
        v.memop_wdata = vb_memop_wdata;
        if ((v_memop_debug == 1'b0)
                && ((mux.memop_type[MemopType_Store] == 1'b0)
                        || (mux.memop_type[MemopType_Release] == 1'b1))) begin
            // Error code of the instruction SC (store with release) should
            // be written into register
            v.tagcnt = vb_tagcnt_next;
            v.reg_waddr = vb_reg_waddr;
            v.reg_wtag = t_tagcnt_wr;
        end
    end else if (i_memop_ready == 1'b1) begin
        v.memop_valid = 1'b0;
        v.memop_debug = 1'b0;
    end

    if (~async_reset && i_nrst == 1'b0) begin
        v = InstrExecute_r_reset;
    end

    wb_rdata1 = vb_rdata1;
    wb_rdata2 = vb_rdata2;

    t_alu_mode[2] = (wv[Instr_XOR]
            || wv[Instr_XORI]
            || wv[Instr_AMOXOR_D]
            || wv[Instr_AMOXOR_W]);
    t_alu_mode[1] = (wv[Instr_OR]
            || wv[Instr_ORI]
            || wv[Instr_AMOOR_D]
            || wv[Instr_AMOOR_W]);
    t_alu_mode[0] = (wv[Instr_AND]
            || wv[Instr_ANDI]
            || wv[Instr_AMOAND_D]
            || wv[Instr_AMOAND_W]);
    wb_alu_mode = t_alu_mode;

    t_addsub_mode[6] = (wv[Instr_AMOMAX_D]
            || wv[Instr_AMOMAX_W]
            || wv[Instr_AMOMAXU_D]
            || wv[Instr_AMOMAXU_W]);
    t_addsub_mode[5] = (wv[Instr_AMOMIN_D]
            || wv[Instr_AMOMIN_W]
            || wv[Instr_AMOMINU_D]
            || wv[Instr_AMOMINU_W]);
    t_addsub_mode[4] = (wv[Instr_SLT]
            || wv[Instr_SLTI]
            || wv[Instr_SLTU]
            || wv[Instr_SLTIU]);
    t_addsub_mode[3] = (wv[Instr_SUB]
            || wv[Instr_SUBW]);
    t_addsub_mode[2] = (wv[Instr_ADD]
            || wv[Instr_ADDI]
            || wv[Instr_ADDW]
            || wv[Instr_ADDIW]
            || wv[Instr_AUIPC]
            || wv[Instr_AMOADD_D]
            || wv[Instr_AMOADD_W]
            || wv[Instr_AMOSWAP_D]
            || wv[Instr_AMOSWAP_W]);
    t_addsub_mode[1] = (wv[Instr_SLTU]
            || wv[Instr_SLTIU]
            || wv[Instr_AMOMINU_D]
            || wv[Instr_AMOMINU_W]
            || wv[Instr_AMOMAXU_D]
            || wv[Instr_AMOMAXU_W]);
    t_addsub_mode[0] = mux.rv32;
    wb_addsub_mode = t_addsub_mode;

    t_shifter_mode[3] = (wv[Instr_SRA]
            || wv[Instr_SRAI]
            || wv[Instr_SRAW]
            || wv[Instr_SRAW]
            || wv[Instr_SRAIW]);
    t_shifter_mode[2] = (wv[Instr_SRL]
            || wv[Instr_SRLI]
            || wv[Instr_SRLW]
            || wv[Instr_SRLIW]);
    t_shifter_mode[1] = (wv[Instr_SLL]
            || wv[Instr_SLLI]
            || wv[Instr_SLLW]
            || wv[Instr_SLLIW]);
    t_shifter_mode[0] = mux.rv32;
    wb_shifter_mode = t_shifter_mode;

    wb_shifter_a1 = vb_rdata1;
    wb_shifter_a2 = vb_rdata2[5: 0];

    if (i_dbg_progbuf_ena == 1'b1) begin
        vb_o_npc = r.dnpc;
    end else begin
        vb_o_npc = r.npc;
    end
    if (r.state == State_Halted) begin
        v_halted = 1'b1;
    end

    o_radr1 = mux.radr1;
    o_radr2 = mux.radr2;
    o_reg_wena = r.reg_write;
    o_reg_waddr = r.reg_waddr;
    o_reg_wtag = r.reg_wtag;
    o_reg_wdata = vb_res;
    o_memop_valid = r.memop_valid;
    o_memop_debug = r.memop_debug;
    o_memop_sign_ext = r.memop_sign_ext;
    o_memop_type = r.memop_type;
    o_memop_size = r.memop_size;
    o_memop_memaddr = r.memop_memaddr;
    o_memop_wdata = r.memop_wdata;
    o_csr_req_valid = v_csr_req_valid;
    o_csr_req_type = r.csr_req_type;
    o_csr_req_addr = r.csr_req_addr;
    o_csr_req_data = r.csr_req_data;
    o_csr_resp_ready = v_csr_resp_ready;
    o_dbg_mem_req_ready = v_dbg_mem_req_ready;
    o_dbg_mem_req_error = v_dbg_mem_req_error;
    o_valid = r.valid;
    o_pc = r.pc;
    o_npc = vb_o_npc;
    o_instr = r.instr;
    o_call = r.call;
    o_ret = r.ret;
    o_jmp = r.jmp;
    o_halted = v_halted;

    // Debug rtl only:!!
    for (int i = 0; i < INTREGS_TOTAL; i++) begin
        tag_expected[i] = r.tagcnt[(CFG_REG_TAG_WIDTH * i) +: CFG_REG_TAG_WIDTH];
    end

    rin = v;
end: comb_proc

generate
    if (async_reset) begin: async_rst_gen

        always_ff @(posedge i_clk, negedge i_nrst) begin: rg_proc
            if (i_nrst == 1'b0) begin
                r <= InstrExecute_r_reset;
            end else begin
                r <= rin;
            end
        end: rg_proc


    end: async_rst_gen
    else begin: no_rst_gen

        always_ff @(posedge i_clk) begin: rg_proc
            r <= rin;
        end: rg_proc

    end: no_rst_gen
endgenerate

endmodule: InstrExecute
