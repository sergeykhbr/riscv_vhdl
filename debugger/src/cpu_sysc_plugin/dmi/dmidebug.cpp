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

#include "dmidebug.h"
#include <riscv-isa.h>

namespace debugger {

DmiDebug::DmiDebug(IFace *parent, sc_module_name name) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    o_haltreq("o_haltreq"),
    o_resumereq("o_resumereq"),
    i_halted("i_halted") {
    iparent_ = parent;
    setBaseAddress(DMI_BASE_ADDRESS);
    setLength(4096);

    SC_METHOD(comb);
    sensitive << bus_req_event_;
    sensitive << i_nrst;
    sensitive << i_halted;
    sensitive << r.regidx;
    sensitive << r.wdata;
    sensitive << r.regwr;
    sensitive << r.regrd;
    sensitive << r.haltreq;
    sensitive << r.resumereq;
    sensitive << r.data0;
    sensitive << r.data1;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    memset(&bank_, 0, sizeof(bank_));
    memset(&bankaccess_, 0, sizeof(bankaccess_));
}

DmiDebug::~DmiDebug() {
}

void DmiDebug::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (i_vcd) {
    }
    if (o_vcd) {
        sc_trace(o_vcd, o_haltreq, o_haltreq.name());
        sc_trace(o_vcd, o_resumereq, o_resumereq.name());
        sc_trace(o_vcd, i_halted, i_halted.name());

        std::string pn(name());
        sc_trace(o_vcd, r.regidx, pn + ".r_regidx");
        sc_trace(o_vcd, r.wdata, pn + ".r_wdata");
        sc_trace(o_vcd, r.regwr, pn + ".r_regwr");
        sc_trace(o_vcd, r.regrd, pn + ".r_regrd");
        sc_trace(o_vcd, r.haltreq, pn + ".r_haltreq");
    }
}

ETransStatus DmiDebug::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();
    if ((off + trans->xsize) > sizeof(bank_)) {
        return TRANS_ERROR;
    }

    if (trans->action == MemAction_Read) {
        readreg(off >> 2, trans->rpayload.b8);
        if (trans->xsize == 8) {
            readreg((off + 1) >> 2, &trans->rpayload.b8[4]);
        }
    } else {
        if (trans->wstrb & 0x00FF) {
            writereg(off >> 2, trans->wpayload.b8);
        }
        if (trans->wstrb & 0xFF00) {
            writereg((off + 4) >> 2, &trans->wpayload.b8[4]);
        }
    }
    return TRANS_OK;
}

void DmiDebug::readreg(uint64_t idx, uint8_t *buf) {
    memcpy(buf, &bank_.b32[idx], sizeof(uint32_t));
    bankaccess_[idx] |= BANK_REG_READ;
}

void DmiDebug::writereg(uint64_t idx, uint8_t *buf) {
    memcpy(&bank_.b32[idx], buf, sizeof(uint32_t));
    bankaccess_[idx] |= BANK_REG_WRITE;
}


void DmiDebug::comb() {
    sc_uint<7> vb_regidx;
    sc_uint<32> vb_wdata;
    bool v_regwr;
    bool v_regrd;

    v = r;

    vb_regidx = 0;
    vb_wdata = 0;
    v_regwr = 0;
    v_regrd = 0;

    for (uint64_t i = 0; i < sizeof(bankaccess_); i++) {
        if (bankaccess_[i] & BANK_REG_READ) {
            vb_regidx = i;
            v_regrd = 1;
            break;
        } else if (bankaccess_[i] & BANK_REG_WRITE) {
            vb_regidx = i;
            v_regwr = 1;
            vb_wdata = bank_.b32[i];
            break;
        }
    }
    v.regidx = vb_regidx;
    v.wdata = vb_wdata;
    v.regwr = v_regwr;
    v.regrd = v_regrd;

    if (r.haltreq && i_halted.read()) {
        v.haltreq = 0;
    }
    if (r.resumereq && !i_halted.read()) {
        v.resumereq = 0;
    }


    switch (r.regidx.read()) {
    case 0x04:  // arg0[31:0]
        if (r.regwr) {
            v.data0 = r.wdata;
        }
        break;
    case 0x05:  // arg0[63:32]
        if (r.regwr) {
            v.data1 = r.wdata;
        }
        break;
    case 0x17:  // command
        if (r.regwr) {
            
        }
        break;
    case 0x10:  // dmcontrol
        if (r.regwr) {
            v.haltreq = r.wdata.read()[31] && !i_halted.read();
            v.resumereq = r.wdata.read()[30] && i_halted.read();
        }
        break;
    default:;
    }

    if (!i_nrst) {
        R_RESET(v);
    }

    o_haltreq = r.haltreq;
    o_resumereq = r.resumereq;
}

void DmiDebug::registers() {
    r = v;

    if (r.regrd.read()) {
        bankaccess_[r.regidx.read().to_int()] &= ~BANK_REG_READ;
        bus_req_event_.notify(1, SC_NS);
    } else if (r.regwr.read()) {
        bankaccess_[r.regidx.read().to_int()] &= ~BANK_REG_WRITE;
        bus_req_event_.notify(1, SC_NS);
    } else {
        for (uint64_t i = 0; i < sizeof(bankaccess_); i++) {
            if (bankaccess_[i]) {
                bus_req_event_.notify(1, SC_NS);
                break;
            }
        }
    }
}

}  // namespace debugger

