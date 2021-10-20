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

DmiDebug::DmiDebug(IFace *parent, sc_module_name name, bool async_reset)
    : sc_module(name),
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
    async_reset_ = async_reset;
    setBaseAddress(DMI_BASE_ADDRESS);
    setLength(4096);

    SC_METHOD(comb);
    sensitive << bus_req_event_;
    sensitive << i_nrst;
    sensitive << i_halted;
    sensitive << i_dport_req_ready;
    sensitive << i_dport_resp_valid;
    sensitive << i_dport_rdata;
    sensitive << w_i_trst;
    sensitive << w_i_tck;
    sensitive << w_i_tms;
    sensitive << w_i_tdi;
    sensitive << w_cdc_dmi_req_valid;
    sensitive << w_cdc_dmi_req_write;
    sensitive << wb_cdc_dmi_req_addr;
    sensitive << wb_cdc_dmi_req_data;
    sensitive << w_cdc_dmi_reset;
    sensitive << w_cdc_dmi_hardreset;

    sensitive << r.bus_req_valid;
    sensitive << r.bus_req_addr;
    sensitive << r.bus_req_write;
    sensitive << r.bus_req_wdata;
    sensitive << r.bus_resp_valid;
    sensitive << r.bus_resp_data;
    sensitive << r.jtag_resp_data;
    sensitive << r.regidx;
    sensitive << r.wdata;
    sensitive << r.regwr;
    sensitive << r.regrd;
    sensitive << r.state;
    sensitive << r.haltreq;
    sensitive << r.resumereq;
    sensitive << r.resumeack;
    sensitive << r.dmactive;
    sensitive << r.hartsel;
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
    tap_->i_trst(w_i_trst);
    tap_->i_tck(w_i_tck);
    tap_->i_tms(w_i_tms);
    tap_->i_tdi(w_i_tdi);
    tap_->o_tdo(w_o_tdo);
    tap_->o_dmi_req_valid(w_tap_dmi_req_valid);
    tap_->o_dmi_req_write(w_tap_dmi_req_write);
    tap_->o_dmi_req_addr(wb_tap_dmi_req_addr);
    tap_->o_dmi_req_data(wb_tap_dmi_req_data);
    tap_->i_dmi_resp_data(wb_jtag_dmi_resp_data);
    tap_->i_dmi_busy(w_jtag_dmi_busy);
    tap_->i_dmi_error(w_jtag_dmi_error);
    tap_->o_dmi_reset(w_tap_dmi_reset);
    tap_->o_dmi_hardreset(w_tap_dmi_hardreset);
    tap_->o_trst(w_tap_trst);

    cdc_ = new JtagCDC("cdc", async_reset);
    cdc_->i_nrst(i_nrst);
    cdc_->i_clk(i_clk);
    cdc_->i_dmi_req_valid(w_tap_dmi_req_valid);
    cdc_->i_dmi_req_write(w_tap_dmi_req_write);
    cdc_->i_dmi_req_addr(wb_tap_dmi_req_addr);
    cdc_->i_dmi_req_data(wb_tap_dmi_req_data);
    cdc_->i_dmi_reset(w_tap_dmi_reset);
    cdc_->i_dmi_hardreset(w_tap_dmi_hardreset);
    cdc_->o_dmi_req_valid(w_cdc_dmi_req_valid);
    cdc_->i_dmi_req_ready(w_cdc_dmi_req_ready);
    cdc_->o_dmi_req_write(w_cdc_dmi_req_write);
    cdc_->o_dmi_req_addr(wb_cdc_dmi_req_addr);
    cdc_->o_dmi_req_data(wb_cdc_dmi_req_data);
    cdc_->o_dmi_reset(w_cdc_dmi_reset);
    cdc_->o_dmi_hardreset(w_cdc_dmi_hardreset);

    trst_ = 0;
    bus_req_valid_ = 0;
    bus_req_addr_ = 0;
    bus_req_write_ = 0;
    bus_req_wdata_ = 0;
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
    cdc_->generateVCD(i_vcd, o_vcd);
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
        sc_trace(o_vcd, w_i_tck, pn + ".i_tck");
        sc_trace(o_vcd, w_i_tms, pn + ".i_tms");
        sc_trace(o_vcd, w_i_tdi, pn + ".i_tdi");
        sc_trace(o_vcd, w_cdc_dmi_req_valid, pn + ".w_cdc_dmi_req_valid");
        sc_trace(o_vcd, w_cdc_dmi_req_write, pn + ".w_cdc_dmi_req_write");
        sc_trace(o_vcd, wb_cdc_dmi_req_addr, pn + ".wb_cdc_dmi_req_addr");
        sc_trace(o_vcd, wb_cdc_dmi_req_data, pn + ".wb_cdc_dmi_req_data");
        sc_trace(o_vcd, r.bus_jtag, pn + ".r_bus_jtag");
        sc_trace(o_vcd, r.bus_req_valid, pn + ".r_bus_req_valid");
        sc_trace(o_vcd, r.bus_req_addr, pn + ".r_bus_req_addr");
        sc_trace(o_vcd, r.bus_req_wdata, pn + ".r_bus_req_wdata");
        sc_trace(o_vcd, r.bus_req_write, pn + ".r_bus_req_write");
        sc_trace(o_vcd, r.bus_resp_valid, pn + ".r_bus_resp_valid");
        sc_trace(o_vcd, r.bus_resp_data, pn + ".r_bus_resp_data");
        sc_trace(o_vcd, r.jtag_resp_data, pn + ".r_jtag_resp_data");
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.regidx, pn + ".r_regidx");
        sc_trace(o_vcd, r.wdata, pn + ".r_wdata");
        sc_trace(o_vcd, r.regwr, pn + ".r_regwr");
        sc_trace(o_vcd, r.regrd, pn + ".r_regrd");
        sc_trace(o_vcd, r.resumeack, pn + ".r_resumeack");
        sc_trace(o_vcd, r.haltreq, pn + ".r_haltreq");
        sc_trace(o_vcd, r.dmactive, pn + ".r_dmactive");
        sc_trace(o_vcd, r.hartsel, pn + ".r_hartsel");
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
    trans->rpayload.b32[0] = bus_resp_data_;
    return TRANS_OK;
}

void DmiDebug::readreg(uint64_t idx) {
    bus_req_addr_ = idx;
    bus_req_write_ = 0;
    bus_req_valid_ = 1;
}

void DmiDebug::writereg(uint64_t idx, uint32_t w32) {
    bus_req_addr_ = idx;
    bus_req_wdata_ = w32;
    bus_req_write_ = 1;
    bus_req_valid_ = 1;
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
    return w_o_tdo.read();
}


void DmiDebug::comb() {
    v = r;
    sc_uint<32> vb_resp_data;
    bool v_resp_valid;

    vb_resp_data = 0;
    v_resp_valid = 0;
    w_cdc_dmi_req_ready = 0;

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
        if (w_cdc_dmi_req_valid) {
            w_cdc_dmi_req_ready = 1;
            v.bus_jtag = 1;
            v.state = STATE_DMI_ACCESS;
            v.regidx = (0, wb_cdc_dmi_req_addr);    /// !!!!!!!!!!!
            v.wdata = wb_cdc_dmi_req_data;
            v.regwr = w_cdc_dmi_req_write;
            v.regrd = !w_cdc_dmi_req_write;
        } else if (r.bus_req_valid.read()) {
            v.bus_jtag = 0;
            v.bus_req_valid = 0;
            v.state = STATE_DMI_ACCESS;
            v.regidx = r.bus_req_addr;
            v.wdata = r.bus_req_wdata;
            v.regwr = r.bus_req_write;
            v.regrd = !r.bus_req_write;
        }
        break;
    case STATE_DMI_ACCESS:
        v_resp_valid = 1;
        v.state = STATE_IDLE;
        switch (r.regidx.read()) {
        case 0x04:  // arg0[31:0]
            vb_resp_data = r.data0;
            if (r.regwr) {
                v.data0 = r.wdata;
            }
            break;
        case 0x05:  // arg0[63:32]
            vb_resp_data = r.data1;
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
            vb_resp_data[0] = r.dmactive;
            //vb_resp_data(25, 16) = (0, r.hartsel);      // hartsello
            if (r.regwr) {
                v.haltreq = r.wdata.read()[31] && !i_halted.read();
                v.resumereq = r.wdata.read()[30] && i_halted.read();
                if (r.wdata.read()[30] && i_halted.read()) {
                    v.resumeack = 0;
                }
                v.dmactive = r.wdata.read()[0];
                v.hartsel = r.wdata.read()(16 + CFG_LOG2_CPU_MAX - 1, 16);
            }
            break;
        case 0x11:  // dmstatus
            vb_resp_data[17] = r.resumeack;                 // allresumeack
            vb_resp_data[16] = r.resumeack;                 // anyresumeack
            vb_resp_data[15] = 0;//not i_dporto(hsel).available; -- allnonexistent
            vb_resp_data[14] = 0;//not i_dporto(hsel).available; -- anynonexistent
            vb_resp_data[13] = 0;//not i_dporto(hsel).available; -- allunavail
            vb_resp_data[12] = 0;//not i_dporto(hsel).available; -- anyunavail
            vb_resp_data[11] = !i_halted;//not i_dporto(hsel).halted and i_dporto(hsel).available; -- allrunning:
            vb_resp_data[10] = !i_halted;//not i_dporto(hsel).halted and i_dporto(hsel).available; -- anyrunning:
            vb_resp_data[9] = i_halted;//i_dporto(hsel).halted and i_dporto(hsel).available;      -- allhalted:
            vb_resp_data[8] = i_halted;//i_dporto(hsel).halted and i_dporto(hsel).available;      // anyhalted:
            vb_resp_data[7] = 1;                   // authenticated:
            vb_resp_data(3, 0) = 0x2;              // version: dbg spec v0.13
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

    if (v_resp_valid) {
        if (!r.bus_jtag) {
            v.bus_resp_data = vb_resp_data;
        } else {
            v.jtag_resp_data = vb_resp_data;
        }
    }
    v.bus_resp_valid = v_resp_valid && !r.bus_jtag;


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

    wb_jtag_dmi_resp_data = r.jtag_resp_data;
    w_jtag_dmi_busy = 0;
    w_jtag_dmi_error = 0;
}

void DmiDebug::registers() {
    r = v;

    if (bus_req_valid_ && r.state.read() == STATE_IDLE) {
        bus_req_valid_ = 0;
        r.bus_req_valid = 1;
        r.bus_req_addr = bus_req_addr_;
        r.bus_req_wdata = bus_req_wdata_;
        r.bus_req_write = bus_req_write_;
        r.bus_resp_valid = 0;
        r.bus_resp_data = 0;
    }

    if (r.bus_resp_valid) {
        bus_resp_data_ = r.bus_resp_data.read();
        RISCV_event_set(&event_tap_resp_valid_);
    }

    w_i_trst = trst_;
    w_i_tck = tck_;
    w_i_tms = tms_;
    w_i_tdi = tdi_;
    if (dtm_scaler_cnt_ < 3) {
        if (++dtm_scaler_cnt_ == 3) {
            RISCV_event_set(&event_dtm_ready_);
        }
    }
}

}  // namespace debugger

