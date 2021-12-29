/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <api_core.h>
#include "decoder.h"

namespace debugger {

InstrDecoder::InstrDecoder(sc_module_name name_, bool async_reset,
    bool fpu_ena) : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_f_pc("i_f_pc"),
    i_f_instr("i_f_instr"),
    i_instr_load_fault("i_instr_load_fault"),
    i_instr_executable("i_instr_executable"),
    i_e_npc("i_e_npc"),
    o_radr1("o_radr1"),
    o_radr2("o_radr2"),
    o_waddr("o_waddr"),
    o_csr_addr("o_csr_addr"),
    o_imm("o_imm"),
    i_flush_pipeline("i_flush_pipeline"),
    i_progbuf_ena("i_progbuf_ena"),
    o_pc("o_pc"),
    o_instr("o_instr"),
    o_memop_store("o_memop_store"),
    o_memop_load("o_memop_load"),
    o_memop_sign_ext("o_memop_sign_ext"),
    o_memop_size("o_memop_size"),
    o_rv32("o_rv32"),
    o_compressed("o_compressed"),
    o_amo("o_amo"),
    o_f64("o_f64"),
    o_unsigned_op("o_unsigned_op"),
    o_isa_type("o_isa_type"),
    o_instr_vec("o_instr_vec"),
    o_exception("o_exception"),
    o_instr_load_fault("o_instr_load_fault"),
    o_instr_executable("o_instr_executable"),
    o_progbuf_ena("o_progbuf_ena") {
    async_reset_ = async_reset;
    fpu_ena_ = fpu_ena;

    char tstr[256];

    for (int i = 0; i < DEC_NUM; i++) {
        RISCV_sprintf(tstr, sizeof(tstr), "rv%d", 2*i);
        rv[i] = new DecoderRv(tstr, async_reset, fpu_ena);

        rv[i]->i_clk(i_clk);
        rv[i]->i_nrst(i_nrst);
        rv[i]->i_flush_pipeline(i_flush_pipeline);
        rv[i]->i_progbuf_ena(i_progbuf_ena);
        rv[i]->i_f_pc(wb_f_pc[i]);
        rv[i]->i_f_instr(wb_f_instr[i]);
        rv[i]->i_instr_load_fault(i_instr_load_fault);
        rv[i]->i_instr_executable(i_instr_executable);
        rv[i]->o_radr1(wd[2*i].radr1);
        rv[i]->o_radr2(wd[2*i].radr2);
        rv[i]->o_waddr(wd[2*i].waddr);
        rv[i]->o_csr_addr(wd[2*i].csr_addr);
        rv[i]->o_imm(wd[2*i].imm);
        rv[i]->o_pc(wd[2*i].pc);
        rv[i]->o_instr(wd[2*i].instr);
        rv[i]->o_memop_store(wd[2*i].memop_store);
        rv[i]->o_memop_load(wd[2*i].memop_load);
        rv[i]->o_memop_sign_ext(wd[2*i].memop_sign_ext);
        rv[i]->o_memop_size(wd[2*i].memop_size);
        rv[i]->o_rv32(wd[2*i].rv32);
        rv[i]->o_compressed(wd[2*i].compressed);
        rv[i]->o_amo(wd[2*i].amo);
        rv[i]->o_f64(wd[2*i].f64);
        rv[i]->o_unsigned_op(wd[2*i].unsigned_op);
        rv[i]->o_isa_type(wd[2*i].isa_type);
        rv[i]->o_instr_vec(wd[2*i].instr_vec);
        rv[i]->o_exception(wd[2*i].instr_unimplemented);
        rv[i]->o_instr_load_fault(wd[2*i].instr_load_fault);
        rv[i]->o_instr_executable(wd[2*i].instr_executable);
        rv[i]->o_progbuf_ena(wd[2*i].progbuf_ena);


        RISCV_sprintf(tstr, sizeof(tstr), "rvc%d", 2*i+1);
        rvc[i] = new DecoderRvc(tstr, async_reset);

        rvc[i]->i_clk(i_clk);
        rvc[i]->i_nrst(i_nrst);
        rvc[i]->i_flush_pipeline(i_flush_pipeline);
        rvc[i]->i_progbuf_ena(i_progbuf_ena);
        rvc[i]->i_f_pc(wb_f_pc[i]);
        rvc[i]->i_f_instr(wb_f_instr[i]);
        rvc[i]->i_instr_load_fault(i_instr_load_fault);
        rvc[i]->i_instr_executable(i_instr_executable);
        rvc[i]->o_radr1(wd[2*i+1].radr1);
        rvc[i]->o_radr2(wd[2*i+1].radr2);
        rvc[i]->o_waddr(wd[2*i+1].waddr);
        rvc[i]->o_csr_addr(wd[2*i+1].csr_addr);
        rvc[i]->o_imm(wd[2*i+1].imm);
        rvc[i]->o_pc(wd[2*i+1].pc);
        rvc[i]->o_instr(wd[2*i+1].instr);
        rvc[i]->o_memop_store(wd[2*i+1].memop_store);
        rvc[i]->o_memop_load(wd[2*i+1].memop_load);
        rvc[i]->o_memop_sign_ext(wd[2*i+1].memop_sign_ext);
        rvc[i]->o_memop_size(wd[2*i+1].memop_size);
        rvc[i]->o_rv32(wd[2*i+1].rv32);
        rvc[i]->o_compressed(wd[2*i+1].compressed);
        rvc[i]->o_amo(wd[2*i+1].amo);
        rvc[i]->o_f64(wd[2*i+1].f64);
        rvc[i]->o_unsigned_op(wd[2*i+1].unsigned_op);
        rvc[i]->o_isa_type(wd[2*i+1].isa_type);
        rvc[i]->o_instr_vec(wd[2*i+1].instr_vec);
        rvc[i]->o_exception(wd[2*i+1].instr_unimplemented);
        rvc[i]->o_instr_load_fault(wd[2*i+1].instr_load_fault);
        rvc[i]->o_instr_executable(wd[2*i+1].instr_executable);
        rvc[i]->o_progbuf_ena(wd[2*i+1].progbuf_ena);
    }


    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_f_pc;
    sensitive << i_f_instr;
    sensitive << i_instr_load_fault;
    sensitive << i_instr_executable;
    sensitive << i_e_npc;
    sensitive << i_flush_pipeline;
    sensitive << i_progbuf_ena;
    for (int i = 0; i < DEC_BLOCK; i++) {
        sensitive << wd[i].pc;
        sensitive << wd[i].instr;
        sensitive << wd[i].memop_load;
        sensitive << wd[i].memop_store;
        sensitive << wd[i].memop_sign_ext;
        sensitive << wd[i].memop_size;
        sensitive << wd[i].unsigned_op;
        sensitive << wd[i].rv32;
        sensitive << wd[i].f64;
        sensitive << wd[i].compressed;
        sensitive << wd[i].amo;
        sensitive << wd[i].instr_load_fault;
        sensitive << wd[i].instr_executable;
        sensitive << wd[i].instr_unimplemented;
        sensitive << wd[i].radr1;
        sensitive << wd[i].radr2;
        sensitive << wd[i].waddr;
        sensitive << wd[i].csr_addr;
        sensitive << wd[i].imm;
        sensitive << wd[i].progbuf_ena;
    }
    for (int i = 0; i < FULL_DEC_DEPTH; i++) {
        sensitive << r[i].pc;
        sensitive << r[i].instr;
        sensitive << r[i].memop_load;
        sensitive << r[i].memop_store;
        sensitive << r[i].memop_sign_ext;
        sensitive << r[i].memop_size;
        sensitive << r[i].unsigned_op;
        sensitive << r[i].rv32;
        sensitive << r[i].f64;
        sensitive << r[i].compressed;
        sensitive << r[i].amo;
        sensitive << r[i].instr_load_fault;
        sensitive << r[i].instr_executable;
        sensitive << r[i].instr_unimplemented;
        sensitive << r[i].radr1;
        sensitive << r[i].radr2;
        sensitive << r[i].waddr;
        sensitive << r[i].csr_addr;
        sensitive << r[i].imm;
        sensitive << r[i].progbuf_ena;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

InstrDecoder::~InstrDecoder() {
    for (int i = 0; i < DEC_NUM; i++) {
        delete rv[i];
        delete rvc[i];
    }
}

void InstrDecoder::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_f_pc, i_f_pc.name());
        sc_trace(o_vcd, i_f_instr, i_f_instr.name());
        sc_trace(o_vcd, i_e_npc, i_e_npc.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_isa_type, o_isa_type.name());
        sc_trace(o_vcd, o_instr_vec, o_instr_vec.name());
        sc_trace(o_vcd, o_exception, o_exception.name());
        sc_trace(o_vcd, o_compressed, o_compressed.name());
        sc_trace(o_vcd, o_amo, o_amo.name());
        sc_trace(o_vcd, o_instr_load_fault, o_instr_load_fault.name());
        sc_trace(o_vcd, o_radr1, o_radr1.name());
        sc_trace(o_vcd, o_radr2, o_radr2.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_csr_addr, o_csr_addr.name());
        sc_trace(o_vcd, o_imm, o_imm.name());

        std::string pn(name());
        sc_trace(o_vcd, selidx, pn + ".selidx");
        sc_trace(o_vcd, shift_ena, pn + ".shift_ena");
        sc_trace(o_vcd, wd[0].pc, pn + ".wd0_pc");
    }
}

void InstrDecoder::comb() {
    selidx = 0;
    shift_ena = 0;

    for (int i = 0; i < FULL_DEC_DEPTH; i++) {
        v[i] = r[i];
        wd[DEC_BLOCK + i] = r[i];
    }

    if (i_f_pc.read() != wd[0].pc.read()) {
        shift_ena = 1;
    }

    // Shift decoder buffer when new instruction available
    if (shift_ena) {
        for (int i = 0; i < DEC_BLOCK; i++) {
            v[i] = wd[i];
        }
        for (int i = DEC_BLOCK; i < FULL_DEC_DEPTH; i++) {
            v[i] = r[i - DEC_BLOCK];
        }
    }

    // Select output decoder:
    for (int i = 0; i < (FULL_DEC_DEPTH + DEC_BLOCK)/2; i++) {
        if (i_e_npc == wd[2*i].pc) {
            if (wd[2*i].compressed == 0) {
                selidx = 2*i;
            } else {
                selidx = 2*i+1;
            }
        }
    }

    // generate decoders inputs with offset
    for (int i = 0; i < DEC_NUM; i++) {
        wb_f_pc[i] = i_f_pc.read() + 2*i;
        wb_f_instr[i] = i_f_instr.read()(16*i+31, 16*i);
    }

    if ((!async_reset_ && !i_nrst.read()) || i_flush_pipeline.read() == 1) {
        for (int i = 0; i < FULL_DEC_DEPTH; i++) {
            R_RESET(v[i]);
        }
    }

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
    o_instr_executable = wd[selidx].instr_executable;

    o_radr1 = wd[selidx].radr1;
    o_radr2 = wd[selidx].radr2;
    o_waddr = wd[selidx].waddr;
    o_csr_addr = wd[selidx].csr_addr;
    o_imm = wd[selidx].imm;
    o_progbuf_ena = wd[selidx].progbuf_ena;
}

void InstrDecoder::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < FULL_DEC_DEPTH; i++) {
            R_RESET(r[i]);
        }
    } else {
        for (int i = 0; i < FULL_DEC_DEPTH; i++) {
            r[i] = v[i];
        }
    }
}

}  // namespace debugger

