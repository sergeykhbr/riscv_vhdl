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
    i_halted("i_halted"),
    i_available("i_available"),
    o_hartsel("o_hartsel"),
    o_haltreq("o_haltreq"),
    o_resumereq("o_resumereq"),
    o_resethaltreq("o_resethaltreq"),
    o_hartreset("o_hartreset"),
    o_ndmreset("o_ndmreset"),
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
    sensitive << i_available;
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
    sensitive << r.dmstate;
    sensitive << r.cmdstate;
    sensitive << r.sbastate;
    sensitive << r.haltreq;
    sensitive << r.resumereq;
    sensitive << r.resumeack;
    sensitive << r.hartreset;
    sensitive << r.resethaltreq;
    sensitive << r.ndmreset;
    sensitive << r.dmactive;
    sensitive << r.hartsel;
    sensitive << r.transfer;
    sensitive << r.write;
    sensitive << r.postexec;
    sensitive << r.command;
    sensitive << r.autoexecdata;
    sensitive << r.autoexecprogbuf;
    sensitive << r.cmderr;
    sensitive << r.data0;
    sensitive << r.data1;
    sensitive << r.progbuf_data;
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
        sc_trace(o_vcd, r.dmstate, pn + ".r_dmstate");
        sc_trace(o_vcd, r.cmdstate, pn + ".r_cmdstate");
        sc_trace(o_vcd, r.sbastate, pn + ".r_sbastate");
        sc_trace(o_vcd, r.regidx, pn + ".r_regidx");
        sc_trace(o_vcd, r.wdata, pn + ".r_wdata");
        sc_trace(o_vcd, r.regwr, pn + ".r_regwr");
        sc_trace(o_vcd, r.regrd, pn + ".r_regrd");
        sc_trace(o_vcd, r.resumeack, pn + ".r_resumeack");
        sc_trace(o_vcd, r.hartreset, pn + ".r_hartreset");
        sc_trace(o_vcd, r.resethaltreq, pn + ".r_resethaltreq");
        sc_trace(o_vcd, r.haltreq, pn + ".r_haltreq");
        sc_trace(o_vcd, r.ndmreset, pn + ".r_ndmreset");
        sc_trace(o_vcd, r.dmactive, pn + ".r_dmactive");
        sc_trace(o_vcd, r.hartsel, pn + ".r_hartsel");
        sc_trace(o_vcd, r.postexec, pn + ".r_postexec");
        sc_trace(o_vcd, r.autoexecdata, pn + ".r_autoexecdata");
        sc_trace(o_vcd, r.autoexecprogbuf, pn + ".r_autoexecprogbuf");
        sc_trace(o_vcd, r.cmderr, pn + ".r_cmderr");
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
    sc_uint<CFG_LOG2_CPU_MAX> vb_hartselnext;
    bool v_resp_valid;
    int hsel;

    vb_resp_data = 0;
    v_resp_valid = 0;
    w_cdc_dmi_req_ready = 0;

    vb_hartselnext = r.wdata.read()(16 + CFG_LOG2_CPU_MAX - 1, 16);
    hsel = r.hartsel.read().to_int();

    if (r.haltreq && i_halted.read()[hsel]) {
        v.haltreq = 0;
    }
    if (r.resumereq && !i_halted.read()[hsel]) {
        v.resumereq = 0;
        v.resumeack = 1;
    }


    switch (r.dmstate.read()) {
    case DM_STATE_IDLE:
        if (w_cdc_dmi_req_valid) {
            w_cdc_dmi_req_ready = 1;
            v.bus_jtag = 1;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = wb_cdc_dmi_req_addr;
            v.wdata = wb_cdc_dmi_req_data;
            v.regwr = w_cdc_dmi_req_write;
            v.regrd = !w_cdc_dmi_req_write;
        } else if (r.bus_req_valid.read()) {
            v.bus_jtag = 0;
            v.bus_req_valid = 0;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = r.bus_req_addr;
            v.wdata = r.bus_req_wdata;
            v.regwr = r.bus_req_write;
            v.regrd = !r.bus_req_write;
        }
        break;
    case DM_STATE_ACCESS:
        v_resp_valid = 1;
        v.dmstate = DM_STATE_IDLE;
        if (r.regidx.read() == 0x04) {  // arg0[31:0]
            vb_resp_data = r.data0;
            if (r.regwr) {
                v.data0 = r.wdata;
            }
            if (r.autoexecdata.read()[0]
                && r.cmderr.read() == CMDERR_NONE) {
                v.cmdstate = CMD_STATE_INIT;
            }
        } else if (r.regidx.read() == 0x05) {  // arg0[63:32]
            vb_resp_data = r.data1;
            if (r.regwr) {
                v.data1 = r.wdata;
            }
            if (r.autoexecdata.read()[1]
                && r.cmderr.read() == CMDERR_NONE) {
                v.cmdstate = CMD_STATE_INIT;
            }
        } else if (r.regidx.read() == 0x10) {  // dmcontrol
            vb_resp_data[29] = r.hartreset;                             // reset selected core
            vb_resp_data[26] = 0;                                       // hasel: single selected hart only
            vb_resp_data(16 + CFG_LOG2_CPU_MAX - 1, 16) = r.hartsel;    // hartsello
            vb_resp_data[1] = r.ndmreset;
            vb_resp_data[0] = r.dmactive;
            if (r.regwr) {
                if (r.wdata.read()[31]) {
                    if (i_halted.read()[vb_hartselnext]) {
                        v.cmderr = CMDERR_WRONGSTATE;
                    } else {
                        v.haltreq = 1;
                    }
                } else if (r.wdata.read()[30]) {
                    // resumereq must be ignored if haltreq is set (@see spec)
                    if (i_halted.read()[vb_hartselnext]) {
                        v.resumereq = 1;
                    } else {
                        v.cmderr = CMDERR_WRONGSTATE;
                    }
                }
                v.hartreset = r.wdata.read()[29];
                v.hartsel = r.wdata.read()(16 + CFG_LOG2_CPU_MAX - 1, 16);
                if (r.wdata.read()[3]) {                                // setresethaltreq
                    v.resethaltreq = 1;
                } else if (r.wdata.read()[2]) {                         // clearresethaltreq
                    v.resethaltreq = 0;
                }
                v.ndmreset = r.wdata.read()[1];                         // 1=Reset whole system including all harts
                v.dmactive = r.wdata.read()[0];
            }
        } else if (r.regidx.read() == 0x11) {  // dmstatus
            // Currently selected ONLY. We support only one selected at once 'hasel=0'
            vb_resp_data[17] = r.resumeack;                     // allresumeack
            vb_resp_data[16] = r.resumeack;                     // anyresumeack
            vb_resp_data[15] = !i_available.read()[hsel];       // allnonexistent
            vb_resp_data[14] = !i_available.read()[hsel];       // anynonexistent
            vb_resp_data[13] = !i_available.read()[hsel];       // allunavail
            vb_resp_data[12] = !i_available.read()[hsel];       // anyunavail
            vb_resp_data[11] = !i_halted.read()[hsel] && i_available.read()[hsel]; // allrunning:
            vb_resp_data[10] = !i_halted.read()[hsel] && i_available.read()[hsel]; // anyrunning:
            vb_resp_data[9] = i_halted.read()[hsel] && i_available.read()[hsel];   // allhalted:
            vb_resp_data[8] = i_halted.read()[hsel] && i_available.read()[hsel];   // anyhalted:
            vb_resp_data[7] = 1;                                // authenticated:
            vb_resp_data[5] = 1;                                // hasresethaltreq
            vb_resp_data(3, 0) = 0x2;                           // version: dbg spec v0.13

        } else if (r.regidx.read() == 0x16) {                   // abstractcs
            vb_resp_data(28,24) = CFG_PROGBUF_REG_TOTAL;
            vb_resp_data[12] = r.cmdstate.read().or_reduce();   // busy
            vb_resp_data(10,8) = r.cmderr;
            vb_resp_data(3,0) = CFG_DATA_REG_TOTAL;
            if (r.regwr && r.wdata.read()(10,8).or_reduce()) {
                v.cmderr = 0;
            }
        } else if (r.regidx.read() == 0x17) {                   // command
            if (r.regwr) {
                if (r.cmderr.read() == CMDERR_NONE) {
                    // If cmderr is non-zero, writes to this register are ignores (@see spec)
                    v.command = r.wdata;
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x18) {                   // abstractauto
            vb_resp_data(CFG_DATA_REG_TOTAL - 1, 0) = r.autoexecdata;
            vb_resp_data(16 + CFG_PROGBUF_REG_TOTAL-1, 16) = r.autoexecprogbuf;
            if (r.regwr) {
                v.autoexecdata = r.wdata.read()(CFG_DATA_REG_TOTAL - 1, 0);
                v.autoexecprogbuf = r.wdata.read()(16 + CFG_PROGBUF_REG_TOTAL-1, 16);
            }
        } else if (r.regidx.read()(6, 4) == 0x02) {             // progbuf[n]
            int tidx = r.regidx.read()(6, 4).to_int();
            vb_resp_data = r.progbuf_data.read()(32*tidx+31, 32*tidx);
            if (r.regwr) {
                sc_biguint<CFG_PROGBUF_REG_TOTAL*32> t2 = r.progbuf_data;
                t2(32*tidx+31, 32*tidx) = r.wdata.read();
                v.progbuf_data = t2;
            }
            if (r.autoexecprogbuf.read()[tidx]
                && r.cmderr.read() == CMDERR_NONE) {
                v.cmdstate = CMD_STATE_INIT;
            }
        } else if (r.regidx.read() == 0x40) {                   // haltsum0
            vb_resp_data(CFG_CPU_MAX-1, 0) = i_halted;
        }
        break;
    default:;
    }

    // Abstract Command executor state machine:
    switch (r.cmdstate.read()) {
    case CMD_STATE_INIT:
        v.cmdstate = CMD_STATE_REQUEST;
        v.cmderr = CMDERR_BUSY;
        v.dport_req_valid = 1;
        v.transfer = r.command.read()[17];
        v.write = r.command.read()[16];
        v.dport_write = r.command.read()[17] & r.command.read()[16];
        v.dport_addr = r.command.read()(15, 0);
        v.dport_wdata = (r.data1, r.data0);
        v.dport_wstrb = (r.command.read()[21] & r.command.read()[20],
                            r.command.read()[21]);
        break;
    case CMD_STATE_REQUEST:
        if (i_dport_req_ready.read()) {
            v.cmdstate = CMD_STATE_RESPONSE;
            v.dport_req_valid = 0;
            v.dport_resp_ready = 1;
        }
        break;
    case CMD_STATE_RESPONSE:
        if (i_dport_resp_valid.read()) {
            v.cmderr = CMDERR_NONE;         // TODO check errors
            v.cmdstate = CMD_STATE_IDLE;
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
    default:
        v.dport_req_valid = 0;
        v.dport_resp_ready = 0;
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

    o_hartsel = r.hartsel;
    o_haltreq = r.haltreq;
    o_resumereq = r.resumereq;
    o_resethaltreq = r.resethaltreq;
    o_hartreset = r.hartreset;
    o_ndmreset = r.ndmreset;

    o_dport_req_valid = r.dport_req_valid;
    o_dport_write = r.dport_write;
    o_dport_addr = r.dport_addr;
    o_dport_wdata = r.dport_wdata;
    o_dport_resp_ready = r.dport_resp_ready;

    wb_jtag_dmi_resp_data = r.jtag_resp_data;
    w_jtag_dmi_busy = 0;//r.dmstate.read().or_reduce();
    w_jtag_dmi_error = 0;
}

void DmiDebug::registers() {
    r = v;

    if (bus_req_valid_ && r.dmstate.read() == DM_STATE_IDLE) {
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

