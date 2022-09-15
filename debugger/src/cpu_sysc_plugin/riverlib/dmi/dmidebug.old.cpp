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

#include "dmidebug.old.h"

namespace debugger {

DmiDebug::DmiDebug(IFace *parent, sc_module_name name, bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_trst("i_trst"),
    i_tck("i_tck"),
    i_tms("i_tms"),
    i_tdi("i_tdi"),
    o_tdo("o_tdo"),
    i_bus_req_valid("i_bus_req_valid"),
    o_bus_req_ready("o_bus_req_ready"),
    i_bus_req_addr("i_bus_req_addr"),
    i_bus_req_write("i_bus_req_write"),
    i_bus_req_wdata("i_bus_req_wdata"),
    o_bus_resp_valid("o_bus_resp_valid"),
    i_bus_resp_ready("i_bus_resp_ready"),
    o_bus_resp_rdata("o_bus_resp_rdata"),
    o_ndmreset("o_ndmreset"),
    i_halted("i_halted"),
    i_available("i_available"),
    o_hartsel("o_hartsel"),
    i_dporto("i_dporto"),
    o_dporti("o_dporti"),
    o_progbuf("o_progbuf") {
    iparent_ = parent;
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << bus_req_event_;
    sensitive << i_nrst;
    sensitive << i_halted;
    sensitive << i_available;
    sensitive << i_dporto;
    sensitive << i_trst;
    sensitive << i_tck;
    sensitive << i_tms;
    sensitive << i_tdi;
    sensitive << w_cdc_dmi_req_valid;
    sensitive << w_cdc_dmi_req_write;
    sensitive << wb_cdc_dmi_req_addr;
    sensitive << wb_cdc_dmi_req_data;
    sensitive << w_cdc_dmi_reset;
    sensitive << w_cdc_dmi_hardreset;

    sensitive << i_bus_req_valid;
    sensitive << i_bus_req_addr;
    sensitive << i_bus_req_write;
    sensitive << i_bus_req_wdata;
    sensitive << i_bus_resp_ready;
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
    sensitive << r.cmd_regaccess;
    sensitive << r.cmd_quickaccess;
    sensitive << r.cmd_memaccess;
    sensitive << r.cmd_progexec;
    sensitive << r.cmd_read;
    sensitive << r.cmd_write;
    sensitive << r.postincrement;
    sensitive << r.aamvirtual;
    sensitive << r.command;
    sensitive << r.autoexecdata;
    sensitive << r.autoexecprogbuf;
    sensitive << r.cmderr;
    sensitive << r.data0;
    sensitive << r.data1;
    sensitive << r.data2;
    sensitive << r.data3;
    sensitive << r.progbuf_data;
    sensitive << r.dport_req_valid;
    sensitive << r.dport_addr;
    sensitive << r.dport_wdata;
    sensitive << r.dport_size;
    sensitive << r.dport_resp_ready;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    tap_ = new JtagTap("tap");
    tap_->i_trst(i_trst);
    tap_->i_tck(i_tck);
    tap_->i_tms(i_tms);
    tap_->i_tdi(i_tdi);
    tap_->o_tdo(o_tdo);
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
}

DmiDebug::~DmiDebug() {
    delete tap_;
    delete cdc_;
}

void DmiDebug::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    tap_->generateVCD(i_vcd, o_vcd);
    cdc_->generateVCD(i_vcd, o_vcd);
    if (i_vcd) {
    }
    if (o_vcd) {
        sc_trace(o_vcd, i_halted, i_halted.name());
        sc_trace(o_vcd, i_dporto, i_dporto.name());

        std::string pn(name());
        sc_trace(o_vcd, i_trst, pn + ".i_trst");
        sc_trace(o_vcd, i_tck, pn + ".i_tck");
        sc_trace(o_vcd, i_tms, pn + ".i_tms");
        sc_trace(o_vcd, i_tdi, pn + ".i_tdi");
        sc_trace(o_vcd, w_cdc_dmi_req_valid, pn + ".w_cdc_dmi_req_valid");
        sc_trace(o_vcd, w_cdc_dmi_req_write, pn + ".w_cdc_dmi_req_write");
        sc_trace(o_vcd, wb_cdc_dmi_req_addr, pn + ".wb_cdc_dmi_req_addr");
        sc_trace(o_vcd, wb_cdc_dmi_req_data, pn + ".wb_cdc_dmi_req_data");
        sc_trace(o_vcd, r.bus_jtag, pn + ".r_bus_jtag");
        sc_trace(o_vcd, i_bus_req_valid, i_bus_req_valid.name());
        sc_trace(o_vcd, i_bus_req_addr, i_bus_req_addr.name());
        sc_trace(o_vcd, i_bus_req_wdata, i_bus_req_wdata.name());
        sc_trace(o_vcd, i_bus_req_write, i_bus_req_write.name());
        sc_trace(o_vcd, o_bus_resp_valid, o_bus_resp_valid.name());
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
        sc_trace(o_vcd, r.postincrement, pn + ".r_postincrement");
        sc_trace(o_vcd, r.aamvirtual, pn + ".r_aamvirtual");
        sc_trace(o_vcd, r.cmd_regaccess, pn + ".r_cmd_regaccess");
        sc_trace(o_vcd, r.cmd_quickaccess, pn + ".r_cmd_quickaccess");
        sc_trace(o_vcd, r.cmd_memaccess, pn + ".r_cmd_memaccess");
        sc_trace(o_vcd, r.cmd_progexec, pn + ".r_cmd_progexec");
        sc_trace(o_vcd, r.cmd_read, pn + ".r_cmd_read");
        sc_trace(o_vcd, r.cmd_write, pn + ".r_cmd_write");
        sc_trace(o_vcd, r.autoexecdata, pn + ".r_autoexecdata");
        sc_trace(o_vcd, r.autoexecprogbuf, pn + ".r_autoexecprogbuf");
        sc_trace(o_vcd, r.cmderr, pn + ".r_cmderr");
    }
}

void DmiDebug::comb() {
    v = r;
    dport_in_type vdporti;
    sc_uint<DPortReq_Total> vb_req_type;
    sc_uint<32> vb_resp_data;
    sc_uint<CFG_LOG2_CPU_MAX> vb_hartselnext;
    bool v_resp_valid;
    bool v_bus_req_ready;
    int hsel;
    bool v_cmd_busy;

    vb_resp_data = 0;
    v_resp_valid = 0;
    w_cdc_dmi_req_ready = 0;
    v_bus_req_ready = 0;

    vb_hartselnext = r.wdata.read()(16 + CFG_LOG2_CPU_MAX - 1, 16);
    hsel = r.hartsel.read().to_int();
    v_cmd_busy = r.cmdstate.read().or_reduce();

    if (r.haltreq && i_halted.read()[hsel]) {
        v.haltreq = 0;
    }
    if (r.resumereq && !i_halted.read()[hsel]) {
        v.resumereq = 0;
        v.resumeack = 1;
    }


    switch (r.dmstate.read()) {
    case DM_STATE_IDLE:
        v_bus_req_ready = 1;
        if (w_cdc_dmi_req_valid) {
            v_bus_req_ready = 0;
            w_cdc_dmi_req_ready = 1;
            v.bus_jtag = 1;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = wb_cdc_dmi_req_addr;
            v.wdata = wb_cdc_dmi_req_data;
            v.regwr = w_cdc_dmi_req_write;
            v.regrd = !w_cdc_dmi_req_write;
        } else if (i_bus_req_valid.read()) {
            v.bus_jtag = 0;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = i_bus_req_addr;
            v.wdata = i_bus_req_wdata;
            v.regwr = i_bus_req_write;
            v.regrd = !i_bus_req_write.read();
        }
        break;
    case DM_STATE_ACCESS:
        v_resp_valid = 1;
        v.dmstate = DM_STATE_IDLE;
        if (r.regidx.read() == 0x04) {                                  // arg0[31:0]
            vb_resp_data = r.data0;
            if (r.regwr) {
                v.data0 = r.wdata;
            }
            if (r.autoexecdata.read()[0] && r.cmderr.read() == CMDERR_NONE) {
                if (v_cmd_busy) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x05) {                           // arg0[63:32]
            vb_resp_data = r.data1;
            if (r.regwr) {
                v.data1 = r.wdata;
            }
            if (r.autoexecdata.read()[1] && r.cmderr.read() == CMDERR_NONE) {
                if (v_cmd_busy) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x06) {                           // arg1[31:0]
            vb_resp_data = r.data2;
            if (r.regwr) {
                v.data2 = r.wdata;
            }
            if (r.autoexecdata.read()[2] && r.cmderr.read() == CMDERR_NONE) {
                if (v_cmd_busy) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x07) {                           // arg1[63:32]
            vb_resp_data = r.data3;
            if (r.regwr) {
                v.data3 = r.wdata;
            }
            if (r.autoexecdata.read()[3] && r.cmderr.read() == CMDERR_NONE) {
                if (v_cmd_busy) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x10) {                           // dmcontrol
            vb_resp_data[29] = r.hartreset;                             // hartreset
            vb_resp_data[28] = 0;                                       // ackhavereset
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
            vb_resp_data[22] = 0;                               // impebreak
            vb_resp_data[19] = 0;                               // allhavereset: selected hart reset but not acknowledged
            vb_resp_data[18] = 0;                               // anyhavereset
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

        } else if (r.regidx.read() == 0x12) {                   // hartinfo
            // Not available core should returns 0
            if (i_available.read()[hsel]) {
                vb_resp_data(23, 20) = 0x2;                     // nscratch number of dscratch registers
                vb_resp_data[16] = 0;                           // dataaccess: 0=CSR shadowed;1=memory shadowed
                vb_resp_data(15, 12) = 0x0;                     // datasize
                vb_resp_data(11, 0) = 0x0;                      // dataaddr
            }
        } else if (r.regidx.read() == 0x16) {                   // abstractcs
            vb_resp_data(28,24) = CFG_PROGBUF_REG_TOTAL;
            vb_resp_data[12] = v_cmd_busy;                      // busy
            vb_resp_data(10,8) = r.cmderr;
            vb_resp_data(3,0) = CFG_DATA_REG_TOTAL;
            if (r.regwr && r.wdata.read()(10,8).or_reduce()) {
                v.cmderr = CMDERR_NONE;
            }
        } else if (r.regidx.read() == 0x17) {                   // command
            if (r.regwr) {
                if (r.cmderr.read() == CMDERR_NONE) {
                    // If cmderr is non-zero, writes to this register are ignores (@see spec)
                    if (v_cmd_busy) {
                        v.cmderr = CMDERR_BUSY;
                    } else {
                        v.command = r.wdata;
                        v.cmdstate = CMD_STATE_INIT;
                    }
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
            int tidx = r.regidx.read()(3, 0).to_int();
            vb_resp_data = r.progbuf_data.read()(32*tidx+31, 32*tidx);
            if (r.regwr) {
                sc_biguint<CFG_PROGBUF_REG_TOTAL*32> t2 = r.progbuf_data;
                t2(32*tidx+31, 32*tidx) = r.wdata.read();
                v.progbuf_data = t2;
            }
            if (r.autoexecprogbuf.read()[tidx] && r.cmderr.read() == CMDERR_NONE) {
                if (v_cmd_busy) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x40) {                   // haltsum0
            vb_resp_data(CFG_CPU_MAX-1, 0) = i_halted;
        }
        break;
    default:;
    }

    // Abstract Command executor state machine:
    switch (r.cmdstate.read()) {
    case CMD_STATE_IDLE:
        v.aamvirtual = 0;
        v.postincrement = 0;
        v.cmd_regaccess = 0;
        v.cmd_quickaccess = 0;
        v.cmd_memaccess = 0;
        v.cmd_progexec = 0;
        v.cmd_read = 0;
        v.cmd_write = 0;
        v.dport_req_valid = 0;
        v.dport_resp_ready = 0;
        break;
    case CMD_STATE_INIT:
        v.postincrement = r.command.read()[CmdPostincrementBit];
        if (r.command.read()(31, 24) == 0x0) {
            // Register access command
            if (r.command.read()[CmdTransferBit]) {             // transfer
                v.cmdstate = CMD_STATE_REQUEST;
                v.cmd_regaccess = 1;
                v.cmd_read = !r.command.read()[CmdWriteBit];
                v.cmd_write = r.command.read()[CmdWriteBit];

                v.dport_req_valid = 1;
                v.dport_addr = (0, r.command.read()(15, 0));
                v.dport_wdata = (r.data1, r.data0);
                v.dport_size = r.command.read()(22, 20);
            } else if (r.command.read()[CmdPostexecBit]) {      // no transfer only buffer execution
                v.cmdstate = CMD_STATE_REQUEST;
        
                v.dport_req_valid = 1;
                v.cmd_progexec = 1;
            } else {
                // Empty command: do nothing
                v.cmdstate = CMD_STATE_IDLE;
            }
        } else if (r.command.read()(31, 24) == 0x1) {
            // Quick access command
            if (i_halted.read()[hsel]) {
                v.cmderr = CMDERR_WRONGSTATE;
                v.cmdstate = CMD_STATE_IDLE;
            } else {
                v.haltreq = 1;
                v.cmd_quickaccess = 1;
                v.cmdstate = CMD_STATE_WAIT_HALTED;
            }
        } else if (r.command.read()(31, 24) == 0x2) {
            // Memory access command
            v.cmdstate = CMD_STATE_REQUEST;
            v.cmd_memaccess = 1;
            v.cmd_read = !r.command.read()[CmdWriteBit];
            v.cmd_write = r.command.read()[CmdWriteBit];
            v.aamvirtual = r.command.read()[23];

            v.dport_req_valid = 1;
            v.dport_addr = (r.data3, r.data2);
            v.dport_wdata = (r.data1, r.data0);
            v.dport_size = r.command.read()(22, 20);
        } else {
            // Unsupported command type:
            v.cmdstate = CMD_STATE_IDLE;
        }
        break;
    case CMD_STATE_REQUEST:
        if (i_dporto.read().req_ready) {
            v.cmdstate = CMD_STATE_RESPONSE;
            v.dport_req_valid = 0;
            v.dport_resp_ready = 1;
        }
        break;
    case CMD_STATE_RESPONSE:
        if (i_dporto.read().resp_valid) {
            v.dport_resp_ready = 0;
            if (r.cmd_read) {
                switch (r.command.read()(22,20)) {          // aamsize/aarsize
                case 0:
                    v.data0 = (0, i_dporto.read().rdata(7, 0));
                    v.data1 = 0;
                    break;
                case 1:
                    v.data0 = (0, i_dporto.read().rdata(15, 0));
                    v.data1 = 0;
                    break;
                case 2:
                    v.data0 = i_dporto.read().rdata(31, 0);
                    v.data1 = 0;
                    break;
                case 3:
                    v.data0 = i_dporto.read().rdata(31, 0);
                    v.data1 = i_dporto.read().rdata(63, 32);
                    break;
                default:;
                }
            }
            if (r.postincrement) {
                v.postincrement = 0;
                if (r.command.read()(31, 24) == 0) {
                    // Register access command:
                    sc_uint<32> t1 = r.command;
                    t1(15, 0) = t1(15, 0) + 1;
                    v.command = t1;
                } else if (r.command.read()(31, 24) == 2) {
                    // Memory access command
                    sc_uint<64> arg1 = (r.data3, r.data2);
                    switch (r.command.read()(22,20)) {          // aamsize
                    case 0:
                        arg1 = arg1 + 1;
                        break;
                    case 1:
                        arg1 = arg1 + 2;
                        break;
                    case 2:
                        arg1 = arg1 + 4;
                        break;
                    case 3:
                        arg1 = arg1 + 8;
                        break;
                    default:;
                    }
                    v.data2 = arg1(31, 0);
                    v.data3 = arg1(63, 32);
                }
            }
            if (i_dporto.read().resp_error) {
                v.cmdstate = CMD_STATE_IDLE;
                if (r.cmd_memaccess) {
                    // @spec The abstract command failed due to a bus error
                    //       (e.g. alignment, access size, or timeout).
                    v.cmderr = CMDERR_BUSERROR;
                } else {
                    // @spec  An exception occurred while executing the
                    //        command (e.g. while executing the Program Buffer).
                    v.cmderr = CMDERR_EXCEPTION;
                }
            } else if (r.cmd_regaccess.read() && r.command.read()[CmdPostexecBit]) {
                v.dport_req_valid = 1;
                v.cmd_progexec = 1;
                v.cmd_regaccess = 0;
                v.cmd_write = 0;
                v.cmd_read = 0;
                v.cmdstate = CMD_STATE_REQUEST;
            } else {
                v.cmdstate = CMD_STATE_IDLE;
            }
            if (r.cmd_quickaccess) {
                // fast command continued even if progbuf execution failed (@see spec)
                v.resumereq = 1;
            }
        }
        break;
    case CMD_STATE_WAIT_HALTED:
        if (i_halted.read()[hsel]) {
            v.cmdstate = CMD_STATE_REQUEST;

            v.dport_req_valid = 1;
            v.cmd_progexec = 1;
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
    if (v_resp_valid == 1 && r.bus_jtag == 0) {
        v.bus_resp_valid = 1;
    } else if (i_bus_resp_ready.read() == 1) {
        v.bus_resp_valid = 0;
    }

    vb_req_type[DPortReq_Write] = r.cmd_write;
    vb_req_type[DPortReq_RegAccess] = r.cmd_regaccess;
    vb_req_type[DPortReq_MemAccess] = r.cmd_memaccess;
    vb_req_type[DPortReq_MemVirtual] = r.aamvirtual;
    vb_req_type[DPortReq_Progexec] = r.cmd_progexec;


    if (!i_nrst) {
        R_RESET(v);
    }

    o_ndmreset = r.ndmreset;
    o_hartsel = r.hartsel;
    vdporti.haltreq = r.haltreq;
    vdporti.resumereq = r.resumereq;
    vdporti.resethaltreq = r.resethaltreq;
    vdporti.hartreset = r.hartreset;
    vdporti.req_valid = r.dport_req_valid;
    vdporti.dtype = vb_req_type;
    vdporti.addr = r.dport_addr;
    vdporti.wdata = r.dport_wdata;
    vdporti.size = r.dport_size;
    vdporti.resp_ready = r.dport_resp_ready;
    o_dporti = vdporti;
    o_progbuf = r.progbuf_data;

    wb_jtag_dmi_resp_data = r.jtag_resp_data;
    w_jtag_dmi_busy = 0;//r.dmstate.read().or_reduce();
    w_jtag_dmi_error = 0;

    o_bus_req_ready = v_bus_req_ready;
    o_bus_resp_valid = r.bus_resp_valid;
    o_bus_resp_rdata = r.bus_resp_data;
}

void DmiDebug::registers() {
    r = v;
}

}  // namespace debugger

