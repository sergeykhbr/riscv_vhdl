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

#include "decoder.h"
#include "api_core.h"

namespace debugger {

InstrDecoder::InstrDecoder(sc_module_name name,
                           bool async_reset,
                           bool fpu_ena)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_f_pc("i_f_pc"),
    i_f_instr("i_f_instr"),
    i_instr_load_fault("i_instr_load_fault"),
    i_instr_page_fault_x("i_instr_page_fault_x"),
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
    o_instr_page_fault_x("o_instr_page_fault_x"),
    o_progbuf_ena("o_progbuf_ena") {

    async_reset_ = async_reset;
    fpu_ena_ = fpu_ena;
    for (int i = 0; i < DEC_NUM; i++) {
        rv[i] = 0;
    }
    for (int i = 0; i < DEC_NUM; i++) {
        rvc[i] = 0;
    }

    for (int i = 0; i < DEC_NUM; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "rv%d", i);
        rv[i] = new DecoderRv(tstr,
                               async_reset,
                               fpu_ena);
        rv[i]->i_clk(i_clk);
        rv[i]->i_nrst(i_nrst);
        rv[i]->i_flush_pipeline(i_flush_pipeline);
        rv[i]->i_progbuf_ena(i_progbuf_ena);
        rv[i]->i_f_pc(wb_f_pc[i]);
        rv[i]->i_f_instr(wb_f_instr[i]);
        rv[i]->i_instr_load_fault(i_instr_load_fault);
        rv[i]->i_instr_page_fault_x(i_instr_page_fault_x);
        rv[i]->o_radr1(wd[(2 * i)].radr1);
        rv[i]->o_radr2(wd[(2 * i)].radr2);
        rv[i]->o_waddr(wd[(2 * i)].waddr);
        rv[i]->o_csr_addr(wd[(2 * i)].csr_addr);
        rv[i]->o_imm(wd[(2 * i)].imm);
        rv[i]->o_pc(wd[(2 * i)].pc);
        rv[i]->o_instr(wd[(2 * i)].instr);
        rv[i]->o_memop_store(wd[(2 * i)].memop_store);
        rv[i]->o_memop_load(wd[(2 * i)].memop_load);
        rv[i]->o_memop_sign_ext(wd[(2 * i)].memop_sign_ext);
        rv[i]->o_memop_size(wd[(2 * i)].memop_size);
        rv[i]->o_rv32(wd[(2 * i)].rv32);
        rv[i]->o_compressed(wd[(2 * i)].compressed);
        rv[i]->o_amo(wd[(2 * i)].amo);
        rv[i]->o_f64(wd[(2 * i)].f64);
        rv[i]->o_unsigned_op(wd[(2 * i)].unsigned_op);
        rv[i]->o_isa_type(wd[(2 * i)].isa_type);
        rv[i]->o_instr_vec(wd[(2 * i)].instr_vec);
        rv[i]->o_exception(wd[(2 * i)].instr_unimplemented);
        rv[i]->o_instr_load_fault(wd[(2 * i)].instr_load_fault);
        rv[i]->o_instr_page_fault_x(wd[(2 * i)].instr_page_fault_x);
        rv[i]->o_progbuf_ena(wd[(2 * i)].progbuf_ena);
    }

    for (int i = 0; i < DEC_NUM; i++) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "rvc%d", i);
        rvc[i] = new DecoderRvc(tstr,
                                 async_reset);
        rvc[i]->i_clk(i_clk);
        rvc[i]->i_nrst(i_nrst);
        rvc[i]->i_flush_pipeline(i_flush_pipeline);
        rvc[i]->i_progbuf_ena(i_progbuf_ena);
        rvc[i]->i_f_pc(wb_f_pc[i]);
        rvc[i]->i_f_instr(wb_f_instr[i]);
        rvc[i]->i_instr_load_fault(i_instr_load_fault);
        rvc[i]->i_instr_page_fault_x(i_instr_page_fault_x);
        rvc[i]->o_radr1(wd[((2 * i) + 1)].radr1);
        rvc[i]->o_radr2(wd[((2 * i) + 1)].radr2);
        rvc[i]->o_waddr(wd[((2 * i) + 1)].waddr);
        rvc[i]->o_csr_addr(wd[((2 * i) + 1)].csr_addr);
        rvc[i]->o_imm(wd[((2 * i) + 1)].imm);
        rvc[i]->o_pc(wd[((2 * i) + 1)].pc);
        rvc[i]->o_instr(wd[((2 * i) + 1)].instr);
        rvc[i]->o_memop_store(wd[((2 * i) + 1)].memop_store);
        rvc[i]->o_memop_load(wd[((2 * i) + 1)].memop_load);
        rvc[i]->o_memop_sign_ext(wd[((2 * i) + 1)].memop_sign_ext);
        rvc[i]->o_memop_size(wd[((2 * i) + 1)].memop_size);
        rvc[i]->o_rv32(wd[((2 * i) + 1)].rv32);
        rvc[i]->o_compressed(wd[((2 * i) + 1)].compressed);
        rvc[i]->o_amo(wd[((2 * i) + 1)].amo);
        rvc[i]->o_f64(wd[((2 * i) + 1)].f64);
        rvc[i]->o_unsigned_op(wd[((2 * i) + 1)].unsigned_op);
        rvc[i]->o_isa_type(wd[((2 * i) + 1)].isa_type);
        rvc[i]->o_instr_vec(wd[((2 * i) + 1)].instr_vec);
        rvc[i]->o_exception(wd[((2 * i) + 1)].instr_unimplemented);
        rvc[i]->o_instr_load_fault(wd[((2 * i) + 1)].instr_load_fault);
        rvc[i]->o_instr_page_fault_x(wd[((2 * i) + 1)].instr_page_fault_x);
        rvc[i]->o_progbuf_ena(wd[((2 * i) + 1)].progbuf_ena);
    }

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_f_pc;
    sensitive << i_f_instr;
    sensitive << i_instr_load_fault;
    sensitive << i_instr_page_fault_x;
    sensitive << i_e_npc;
    sensitive << i_flush_pipeline;
    sensitive << i_progbuf_ena;
    for (int i = 0; i < (FULL_DEC_DEPTH + DEC_BLOCK); i++) {
        sensitive << wd[i].pc;
        sensitive << wd[i].isa_type;
        sensitive << wd[i].instr_vec;
        sensitive << wd[i].instr;
        sensitive << wd[i].memop_store;
        sensitive << wd[i].memop_load;
        sensitive << wd[i].memop_sign_ext;
        sensitive << wd[i].memop_size;
        sensitive << wd[i].unsigned_op;
        sensitive << wd[i].rv32;
        sensitive << wd[i].f64;
        sensitive << wd[i].compressed;
        sensitive << wd[i].amo;
        sensitive << wd[i].instr_load_fault;
        sensitive << wd[i].instr_page_fault_x;
        sensitive << wd[i].instr_unimplemented;
        sensitive << wd[i].radr1;
        sensitive << wd[i].radr2;
        sensitive << wd[i].waddr;
        sensitive << wd[i].csr_addr;
        sensitive << wd[i].imm;
        sensitive << wd[i].progbuf_ena;
    }
    for (int i = 0; i < DEC_NUM; i++) {
        sensitive << wb_f_pc[i];
    }
    for (int i = 0; i < DEC_NUM; i++) {
        sensitive << wb_f_instr[i];
    }
    for (int i = 0; i < FULL_DEC_DEPTH; i++) {
        sensitive << r.d[i].pc;
        sensitive << r.d[i].isa_type;
        sensitive << r.d[i].instr_vec;
        sensitive << r.d[i].instr;
        sensitive << r.d[i].memop_store;
        sensitive << r.d[i].memop_load;
        sensitive << r.d[i].memop_sign_ext;
        sensitive << r.d[i].memop_size;
        sensitive << r.d[i].unsigned_op;
        sensitive << r.d[i].rv32;
        sensitive << r.d[i].f64;
        sensitive << r.d[i].compressed;
        sensitive << r.d[i].amo;
        sensitive << r.d[i].instr_load_fault;
        sensitive << r.d[i].instr_page_fault_x;
        sensitive << r.d[i].instr_unimplemented;
        sensitive << r.d[i].radr1;
        sensitive << r.d[i].radr2;
        sensitive << r.d[i].waddr;
        sensitive << r.d[i].csr_addr;
        sensitive << r.d[i].imm;
        sensitive << r.d[i].progbuf_ena;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

InstrDecoder::~InstrDecoder() {
    for (int i = 0; i < DEC_NUM; i++) {
        if (rv[i]) {
            delete rv[i];
        }
    }
    for (int i = 0; i < DEC_NUM; i++) {
        if (rvc[i]) {
            delete rvc[i];
        }
    }
}

void InstrDecoder::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_f_pc, i_f_pc.name());
        sc_trace(o_vcd, i_f_instr, i_f_instr.name());
        sc_trace(o_vcd, i_instr_load_fault, i_instr_load_fault.name());
        sc_trace(o_vcd, i_instr_page_fault_x, i_instr_page_fault_x.name());
        sc_trace(o_vcd, i_e_npc, i_e_npc.name());
        sc_trace(o_vcd, o_radr1, o_radr1.name());
        sc_trace(o_vcd, o_radr2, o_radr2.name());
        sc_trace(o_vcd, o_waddr, o_waddr.name());
        sc_trace(o_vcd, o_csr_addr, o_csr_addr.name());
        sc_trace(o_vcd, o_imm, o_imm.name());
        sc_trace(o_vcd, i_flush_pipeline, i_flush_pipeline.name());
        sc_trace(o_vcd, i_progbuf_ena, i_progbuf_ena.name());
        sc_trace(o_vcd, o_pc, o_pc.name());
        sc_trace(o_vcd, o_instr, o_instr.name());
        sc_trace(o_vcd, o_memop_store, o_memop_store.name());
        sc_trace(o_vcd, o_memop_load, o_memop_load.name());
        sc_trace(o_vcd, o_memop_sign_ext, o_memop_sign_ext.name());
        sc_trace(o_vcd, o_memop_size, o_memop_size.name());
        sc_trace(o_vcd, o_rv32, o_rv32.name());
        sc_trace(o_vcd, o_compressed, o_compressed.name());
        sc_trace(o_vcd, o_amo, o_amo.name());
        sc_trace(o_vcd, o_f64, o_f64.name());
        sc_trace(o_vcd, o_unsigned_op, o_unsigned_op.name());
        sc_trace(o_vcd, o_isa_type, o_isa_type.name());
        sc_trace(o_vcd, o_instr_vec, o_instr_vec.name());
        sc_trace(o_vcd, o_exception, o_exception.name());
        sc_trace(o_vcd, o_instr_load_fault, o_instr_load_fault.name());
        sc_trace(o_vcd, o_instr_page_fault_x, o_instr_page_fault_x.name());
        sc_trace(o_vcd, o_progbuf_ena, o_progbuf_ena.name());
        for (int i = 0; i < FULL_DEC_DEPTH; i++) {
            sc_trace(o_vcd, r.d[i].pc, pn + ".r.d(" + std::to_string(i) + ").pc");
            sc_trace(o_vcd, r.d[i].isa_type, pn + ".r.d(" + std::to_string(i) + ").isa_type");
            sc_trace(o_vcd, r.d[i].instr_vec, pn + ".r.d(" + std::to_string(i) + ").instr_vec");
            sc_trace(o_vcd, r.d[i].instr, pn + ".r.d(" + std::to_string(i) + ").instr");
            sc_trace(o_vcd, r.d[i].memop_store, pn + ".r.d(" + std::to_string(i) + ").memop_store");
            sc_trace(o_vcd, r.d[i].memop_load, pn + ".r.d(" + std::to_string(i) + ").memop_load");
            sc_trace(o_vcd, r.d[i].memop_sign_ext, pn + ".r.d(" + std::to_string(i) + ").memop_sign_ext");
            sc_trace(o_vcd, r.d[i].memop_size, pn + ".r.d(" + std::to_string(i) + ").memop_size");
            sc_trace(o_vcd, r.d[i].unsigned_op, pn + ".r.d(" + std::to_string(i) + ").unsigned_op");
            sc_trace(o_vcd, r.d[i].rv32, pn + ".r.d(" + std::to_string(i) + ").rv32");
            sc_trace(o_vcd, r.d[i].f64, pn + ".r.d(" + std::to_string(i) + ").f64");
            sc_trace(o_vcd, r.d[i].compressed, pn + ".r.d(" + std::to_string(i) + ").compressed");
            sc_trace(o_vcd, r.d[i].amo, pn + ".r.d(" + std::to_string(i) + ").amo");
            sc_trace(o_vcd, r.d[i].instr_load_fault, pn + ".r.d(" + std::to_string(i) + ").instr_load_fault");
            sc_trace(o_vcd, r.d[i].instr_page_fault_x, pn + ".r.d(" + std::to_string(i) + ").instr_page_fault_x");
            sc_trace(o_vcd, r.d[i].instr_unimplemented, pn + ".r.d(" + std::to_string(i) + ").instr_unimplemented");
            sc_trace(o_vcd, r.d[i].radr1, pn + ".r.d(" + std::to_string(i) + ").radr1");
            sc_trace(o_vcd, r.d[i].radr2, pn + ".r.d(" + std::to_string(i) + ").radr2");
            sc_trace(o_vcd, r.d[i].waddr, pn + ".r.d(" + std::to_string(i) + ").waddr");
            sc_trace(o_vcd, r.d[i].csr_addr, pn + ".r.d(" + std::to_string(i) + ").csr_addr");
            sc_trace(o_vcd, r.d[i].imm, pn + ".r.d(" + std::to_string(i) + ").imm");
            sc_trace(o_vcd, r.d[i].progbuf_ena, pn + ".r.d(" + std::to_string(i) + ").progbuf_ena");
        }
    }

    for (int i = 0; i < DEC_NUM; i++) {
        if (rv[i]) {
            rv[i]->generateVCD(i_vcd, o_vcd);
        }
    }
    for (int i = 0; i < DEC_NUM; i++) {
        if (rvc[i]) {
            rvc[i]->generateVCD(i_vcd, o_vcd);
        }
    }
}

void InstrDecoder::comb() {
    int selidx;
    bool shift_ena;

    for (int i = 0; i < FULL_DEC_DEPTH; i++) {
        v.d[i].pc = r.d[i].pc.read();
        v.d[i].isa_type = r.d[i].isa_type.read();
        v.d[i].instr_vec = r.d[i].instr_vec.read();
        v.d[i].instr = r.d[i].instr.read();
        v.d[i].memop_store = r.d[i].memop_store.read();
        v.d[i].memop_load = r.d[i].memop_load.read();
        v.d[i].memop_sign_ext = r.d[i].memop_sign_ext.read();
        v.d[i].memop_size = r.d[i].memop_size.read();
        v.d[i].unsigned_op = r.d[i].unsigned_op.read();
        v.d[i].rv32 = r.d[i].rv32.read();
        v.d[i].f64 = r.d[i].f64.read();
        v.d[i].compressed = r.d[i].compressed.read();
        v.d[i].amo = r.d[i].amo.read();
        v.d[i].instr_load_fault = r.d[i].instr_load_fault.read();
        v.d[i].instr_page_fault_x = r.d[i].instr_page_fault_x.read();
        v.d[i].instr_unimplemented = r.d[i].instr_unimplemented.read();
        v.d[i].radr1 = r.d[i].radr1.read();
        v.d[i].radr2 = r.d[i].radr2.read();
        v.d[i].waddr = r.d[i].waddr.read();
        v.d[i].csr_addr = r.d[i].csr_addr.read();
        v.d[i].imm = r.d[i].imm.read();
        v.d[i].progbuf_ena = r.d[i].progbuf_ena.read();
    }
    selidx = 0;
    shift_ena = 0;

    for (int i = 0; i < FULL_DEC_DEPTH; i++) {
        wd[(DEC_BLOCK + i)] = r.d[i];
    }

    if (i_f_pc.read() != wd[0].pc.read()) {
        shift_ena = 1;
    }

    // Shift decoder buffer when new instruction available
    if (shift_ena == 1) {
        for (int i = 0; i < DEC_BLOCK; i++) {
            v.d[i] = wd[i];
        }
        for (int i = DEC_BLOCK; i < FULL_DEC_DEPTH; i++) {
            v.d[i] = r.d[(i - DEC_BLOCK)];
        }
    }

    // Select output decoder:
    for (int i = 0; i < ((FULL_DEC_DEPTH + DEC_BLOCK) / 2); i++) {
        if (i_e_npc.read() == wd[(2 * i)].pc.read()) {
            if (wd[(2 * i)].compressed.read() == 0) {
                selidx = (2 * i);
            } else {
                selidx = ((2 * i) + 1);
            }
        }
    }

    // generate decoders inputs with offset
    for (int i = 0; i < DEC_NUM; i++) {
        wb_f_pc[i] = (i_f_pc.read() + (2 * i));
        wb_f_instr[i] = i_f_instr.read()((16 * i) + 32 - 1, (16 * i));
    }

    if (((!async_reset_) && (i_nrst.read() == 0)) || (i_flush_pipeline.read() == 1)) {
        InstrDecoder_r_reset(v);
    }

    o_pc = wd[selidx].pc.read();
    o_instr = wd[selidx].instr.read();
    o_memop_load = wd[selidx].memop_load.read();
    o_memop_store = wd[selidx].memop_store.read();
    o_memop_sign_ext = wd[selidx].memop_sign_ext.read();
    o_memop_size = wd[selidx].memop_size.read();
    o_unsigned_op = wd[selidx].unsigned_op.read();
    o_rv32 = wd[selidx].rv32.read();
    o_f64 = wd[selidx].f64.read();
    o_compressed = wd[selidx].compressed.read();
    o_amo = wd[selidx].amo.read();
    o_isa_type = wd[selidx].isa_type.read();
    o_instr_vec = wd[selidx].instr_vec.read();
    o_exception = wd[selidx].instr_unimplemented.read();
    o_instr_load_fault = wd[selidx].instr_load_fault.read();
    o_instr_page_fault_x = wd[selidx].instr_page_fault_x.read();
    o_radr1 = wd[selidx].radr1.read();
    o_radr2 = wd[selidx].radr2.read();
    o_waddr = wd[selidx].waddr.read();
    o_csr_addr = wd[selidx].csr_addr.read();
    o_imm = wd[selidx].imm.read();
    o_progbuf_ena = wd[selidx].progbuf_ena.read();
}

void InstrDecoder::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        InstrDecoder_r_reset(r);
    } else {
        for (int i = 0; i < FULL_DEC_DEPTH; i++) {
            r.d[i].pc = v.d[i].pc.read();
            r.d[i].isa_type = v.d[i].isa_type.read();
            r.d[i].instr_vec = v.d[i].instr_vec.read();
            r.d[i].instr = v.d[i].instr.read();
            r.d[i].memop_store = v.d[i].memop_store.read();
            r.d[i].memop_load = v.d[i].memop_load.read();
            r.d[i].memop_sign_ext = v.d[i].memop_sign_ext.read();
            r.d[i].memop_size = v.d[i].memop_size.read();
            r.d[i].unsigned_op = v.d[i].unsigned_op.read();
            r.d[i].rv32 = v.d[i].rv32.read();
            r.d[i].f64 = v.d[i].f64.read();
            r.d[i].compressed = v.d[i].compressed.read();
            r.d[i].amo = v.d[i].amo.read();
            r.d[i].instr_load_fault = v.d[i].instr_load_fault.read();
            r.d[i].instr_page_fault_x = v.d[i].instr_page_fault_x.read();
            r.d[i].instr_unimplemented = v.d[i].instr_unimplemented.read();
            r.d[i].radr1 = v.d[i].radr1.read();
            r.d[i].radr2 = v.d[i].radr2.read();
            r.d[i].waddr = v.d[i].waddr.read();
            r.d[i].csr_addr = v.d[i].csr_addr.read();
            r.d[i].imm = v.d[i].imm.read();
            r.d[i].progbuf_ena = v.d[i].progbuf_ena.read();
        }
    }
}

}  // namespace debugger

