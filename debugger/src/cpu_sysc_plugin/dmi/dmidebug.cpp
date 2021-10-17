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
    i_halted("i_halted"),
    o_dport_req_valid("o_dport_req_valid"),
    o_dport_write("o_dport_write"),
    o_dport_addr("o_dport_addr"),
    o_dport_wdata("o_dport_wdata"),
    i_dport_req_ready("i_dport_req_ready"),
    o_dport_resp_ready("o_dport_resp_ready"),
    i_dport_resp_valid("i_dport_resp_valid"),
    i_dport_rdata("i_dport_rdata") {
    iparent_ = parent;
    setBaseAddress(DMI_BASE_ADDRESS);
    setLength(4096);

    SC_METHOD(comb);
    sensitive << bus_req_event_;
    sensitive << i_nrst;
    sensitive << i_halted;
    sensitive << i_dport_req_ready;
    sensitive << i_dport_resp_valid;
    sensitive << i_dport_rdata;
    sensitive << w_trst;
    sensitive << w_tck;
    sensitive << w_tms;
    sensitive << w_tdi;
    sensitive << r.tap_request_valid;
    sensitive << r.tap_request_addr;
    sensitive << r.tap_request_write;
    sensitive << r.tap_request_wdata;
    sensitive << r.tap_response_valid;
    sensitive << r.tap_response_rdata;
    sensitive << r.regidx;
    sensitive << r.wdata;
    sensitive << r.regwr;
    sensitive << r.regrd;
    sensitive << r.state;
    sensitive << r.haltreq;
    sensitive << r.resumereq;
    sensitive << r.resumeack;
    sensitive << r.transfer;
    sensitive << r.write;
    sensitive << r.data0;
    sensitive << r.data1;
    sensitive << r.dport_req_valid;
    sensitive << r.dport_write;
    sensitive << r.dport_addr;
    sensitive << r.dport_wdata;
    sensitive << r.dport_wstrb;
    sensitive << r.dport_resp_ready;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    tap_ = new JtagTap("tap");
    tap_->i_nrst(i_nrst);
    tap_->i_trst(w_trst);
    tap_->i_tck(w_tck);
    tap_->i_tms(w_tms);
    tap_->i_tdi(w_tdi);
    tap_->o_tdo(w_tdo);
    tap_->i_dmi_rdata(wb_dmi_rdata);
    tap_->i_dmi_busy(w_dmi_busy);
    tap_->o_dmi_hardreset(w_dmi_hardreset);

    trst_ = 0;
    tap_request_valid = 0;
    tap_request_addr = 0;
    tap_request_write = 0;
    tap_request_wdata = 0;
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s_event_tap_resp_valid_", name);
    RISCV_event_create(&event_tap_resp_valid_, tstr);
    RISCV_sprintf(tstr, sizeof(tstr), "%s_event_dtm_ready", name);
    RISCV_event_create(&event_dtm_ready_, tstr);
}

DmiDebug::~DmiDebug() {
    delete tap_;
    RISCV_event_close(&event_tap_resp_valid_);
    RISCV_event_close(&event_dtm_ready_);
}

void DmiDebug::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    tap_->generateVCD(i_vcd, o_vcd);
    if (i_vcd) {
    }
    if (o_vcd) {
        sc_trace(o_vcd, o_haltreq, o_haltreq.name());
        sc_trace(o_vcd, o_resumereq, o_resumereq.name());
        sc_trace(o_vcd, i_halted, i_halted.name());
        sc_trace(o_vcd, o_dport_req_valid, o_dport_req_valid.name());
        sc_trace(o_vcd, o_dport_write, o_dport_write.name());
        sc_trace(o_vcd, o_dport_addr, o_dport_addr.name());
        sc_trace(o_vcd, o_dport_wdata, o_dport_wdata.name());
        sc_trace(o_vcd, i_dport_req_ready, i_dport_req_ready.name());
        sc_trace(o_vcd, o_dport_resp_ready, o_dport_resp_ready.name());
        sc_trace(o_vcd, i_dport_resp_valid, i_dport_resp_valid.name());
        sc_trace(o_vcd, i_dport_rdata, i_dport_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, w_tck, pn + ".tck");
        sc_trace(o_vcd, w_tms, pn + ".tms");
        sc_trace(o_vcd, w_tdi, pn + ".tdi");
        sc_trace(o_vcd, r.tap_request_valid, pn + ".r_tap_request_valid");
        sc_trace(o_vcd, r.tap_request_addr, pn + ".r_tap_request_addr");
        sc_trace(o_vcd, r.tap_request_wdata, pn + ".r_tap_request_wdata");
        sc_trace(o_vcd, r.tap_request_write, pn + ".r_tap_request_write");
        sc_trace(o_vcd, r.tap_response_valid, pn + ".r_tap_response_valid");
        sc_trace(o_vcd, r.tap_response_rdata, pn + ".r_tap_response_rdata");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.regidx, pn + ".r_regidx");
        sc_trace(o_vcd, r.wdata, pn + ".r_wdata");
        sc_trace(o_vcd, r.regwr, pn + ".r_regwr");
        sc_trace(o_vcd, r.regrd, pn + ".r_regrd");
        sc_trace(o_vcd, r.resumeack, pn + ".r_resumeack");
        sc_trace(o_vcd, r.haltreq, pn + ".r_haltreq");
    }
}

ETransStatus DmiDebug::b_transport(Axi4TransactionType *trans) {
    uint64_t off = trans->addr - getBaseAddress();
    if ((off + trans->xsize) > sizeof(DmiRegBankType)) {
        return TRANS_ERROR;
    }

    // Only 4-bytes requests:
    if (trans->action == MemAction_Read) {
        readreg(off >> 2);
    } else {
        if (trans->wstrb & 0x00FF) {
            writereg(off >> 2, trans->wpayload.b32[0]);
        } else if (trans->wstrb & 0xFF00) {
            writereg((off + 4) >> 2, trans->wpayload.b32[1]);
        }
    }
    RISCV_event_clear(&event_tap_resp_valid_);
    RISCV_event_wait(&event_tap_resp_valid_);
    trans->rpayload.b32[0] = tap_response_rdata;
    return TRANS_OK;
}

void DmiDebug::readreg(uint64_t idx) {
    tap_request_addr = idx;
    tap_request_write = 0;
    tap_request_valid = 1;
}

void DmiDebug::writereg(uint64_t idx, uint32_t w32) {
    tap_request_addr = idx;
    tap_request_wdata = w32;
    tap_request_write = 1;
    tap_request_valid = 1;
}

void DmiDebug::resetTAP() {
    trst_ = 1;
    dtm_scaler_cnt_ = 0;
    RISCV_event_clear(&event_dtm_ready_);
    RISCV_event_wait(&event_dtm_ready_);
    trst_ = 0;
}

void DmiDebug::setPins(char tck, char tms, char tdi) {
    tck_ = tck;
    tms_ = tms;
    tdi_ = tdi;
    dtm_scaler_cnt_ = 0;

    RISCV_event_clear(&event_dtm_ready_);
    RISCV_event_wait(&event_dtm_ready_);
}

bool DmiDebug::getTDO() {
    return w_tdo.read();
}


void DmiDebug::comb() {
    v = r;
    sc_uint<32> vb_tap_response_rdata;
    bool v_tap_response_valid;

    vb_tap_response_rdata = 0;
    v_tap_response_valid = 0;


    if (r.haltreq && i_halted.read()) {
        v.haltreq = 0;
    }
    if (r.resumereq && !i_halted.read()) {
        v.resumereq = 0;
        v.resumeack = 1;
    }


    switch (r.state.read()) {
    case STATE_IDLE:
        v.dport_req_valid = 0;
        v.dport_resp_ready = 0;
        v.transfer = 0;
        v.write = 0;
        if (r.tap_request_valid.read()) {
            v.tap_request_valid = 0;
            v.state = STATE_DMI_ACCESS;
            v.regidx = r.tap_request_addr;
            v.wdata = r.tap_request_wdata;
            v.regwr = r.tap_request_write;
            v.regrd = !r.tap_request_write;
        }
        break;
    case STATE_DMI_ACCESS:
        v_tap_response_valid = 1;
        v.state = STATE_IDLE;
        switch (r.regidx.read()) {
        case 0x04:  // arg0[31:0]
            vb_tap_response_rdata = r.data0;
            if (r.regwr) {
                v.data0 = r.wdata;
            }
            break;
        case 0x05:  // arg0[63:32]
            vb_tap_response_rdata = r.data1;
            if (r.regwr) {
                v.data1 = r.wdata;
            }
            break;
        case 0x17:  // command
            if (r.regwr) {
                v.state = STATE_CPU_REQUEST;
                v.dport_req_valid = 1;
                v.transfer = r.wdata.read()[17];
                v.write = r.wdata.read()[16];
                v.dport_write = r.wdata.read()[17] & r.wdata.read()[16];
                v.dport_addr = r.wdata.read()(15, 0);
                v.dport_wdata = (r.data1, r.data0);
                v.dport_wstrb = (r.wdata.read()[21] & r.wdata.read()[20],
                                 r.wdata.read()[21]);
            }
            break;
        case 0x10:  // dmcontrol
            if (r.regwr) {
                v.haltreq = r.wdata.read()[31] && !i_halted.read();
                v.resumereq = r.wdata.read()[30] && i_halted.read();
                if (r.wdata.read()[30] && i_halted.read()) {
                    v.resumeack = 0;
                }
            }
            break;
        case 0x11:  // dmstatus
            vb_tap_response_rdata[17] = r.resumeack;                 // allresumeack
            vb_tap_response_rdata[16] = r.resumeack;                 // anyresumeack
            vb_tap_response_rdata[15] = 0;//not i_dporto(hsel).available; -- allnonexistent
            vb_tap_response_rdata[14] = 0;//not i_dporto(hsel).available; -- anynonexistent
            vb_tap_response_rdata[13] = 0;//not i_dporto(hsel).available; -- allunavail
            vb_tap_response_rdata[12] = 0;//not i_dporto(hsel).available; -- anyunavail
            vb_tap_response_rdata[11] = !i_halted;//not i_dporto(hsel).halted and i_dporto(hsel).available; -- allrunning:
            vb_tap_response_rdata[10] = !i_halted;//not i_dporto(hsel).halted and i_dporto(hsel).available; -- anyrunning:
            vb_tap_response_rdata[9] = i_halted;//i_dporto(hsel).halted and i_dporto(hsel).available;      -- allhalted:
            vb_tap_response_rdata[8] = i_halted;//i_dporto(hsel).halted and i_dporto(hsel).available;      // anyhalted:
            vb_tap_response_rdata[7] = 1;                   // authenticated:
            vb_tap_response_rdata(3, 0) = 0x2;              // version: dbg spec v0.13
            break;
        default:;
        }
        break;
    case STATE_CPU_REQUEST:
        if (i_dport_req_ready.read()) {
            v.state = STATE_CPU_RESPONSE;
            v.dport_req_valid = 0;
            v.dport_resp_ready = 1;
        }
        break;
    case STATE_CPU_RESPONSE:
        if (i_dport_resp_valid.read()) {
            v.state = STATE_IDLE;
            v.dport_resp_ready = 0;
            if (r.transfer && !r.write) {
                if (r.dport_wstrb.read()[1]) {
                    v.data1 = i_dport_rdata.read()(63, 32);
                }
                if (r.dport_wstrb.read()[0]) {
                    v.data0 = i_dport_rdata.read()(31, 0);
                }
            }
        }
        break;
    default:;
    }

    v.tap_response_rdata = vb_tap_response_rdata;
    v.tap_response_valid = v_tap_response_valid;


    if (!i_nrst) {
        R_RESET(v);
    }

    o_haltreq = r.haltreq;
    o_resumereq = r.resumereq;

    o_dport_req_valid = r.dport_req_valid;
    o_dport_write = r.dport_write;
    o_dport_addr = r.dport_addr;
    o_dport_wdata = r.dport_wdata;
    o_dport_resp_ready = r.dport_resp_ready;
}

void DmiDebug::registers() {
    r = v;

    if (tap_request_valid && r.state.read() == STATE_IDLE) {
        tap_request_valid = 0;
        r.tap_request_valid = 1;
        r.tap_request_addr = tap_request_addr;
        r.tap_request_wdata = tap_request_wdata;
        r.tap_request_write = tap_request_write;
        r.tap_response_valid = 0;
        r.tap_response_rdata = 0;
    }

    if (r.tap_response_valid) {
        tap_response_rdata = r.tap_response_rdata.read();
        RISCV_event_set(&event_tap_resp_valid_);
    }

    w_trst = trst_;
    w_tck = tck_;
    w_tms = tms_;
    w_tdi = tdi_;
    if (dtm_scaler_cnt_ < 3) {
        if (++dtm_scaler_cnt_ == 3) {
            RISCV_event_set(&event_dtm_ready_);
        }
    }
}

}  // namespace debugger

