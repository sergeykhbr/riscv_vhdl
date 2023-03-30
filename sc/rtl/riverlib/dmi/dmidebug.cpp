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

#include "dmidebug.h"
#include "api_core.h"

namespace debugger {

dmidebug::dmidebug(sc_module_name name,
                   bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_trst("i_trst"),
    i_tck("i_tck"),
    i_tms("i_tms"),
    i_tdi("i_tdi"),
    o_tdo("o_tdo"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_ndmreset("o_ndmreset"),
    i_halted("i_halted"),
    i_available("i_available"),
    o_hartsel("o_hartsel"),
    o_haltreq("o_haltreq"),
    o_resumereq("o_resumereq"),
    o_resethaltreq("o_resethaltreq"),
    o_hartreset("o_hartreset"),
    o_dport_req_valid("o_dport_req_valid"),
    o_dport_req_type("o_dport_req_type"),
    o_dport_addr("o_dport_addr"),
    o_dport_wdata("o_dport_wdata"),
    o_dport_size("o_dport_size"),
    i_dport_req_ready("i_dport_req_ready"),
    o_dport_resp_ready("o_dport_resp_ready"),
    i_dport_resp_valid("i_dport_resp_valid"),
    i_dport_resp_error("i_dport_resp_error"),
    i_dport_rdata("i_dport_rdata"),
    o_progbuf("o_progbuf") {

    async_reset_ = async_reset;
    cdc = 0;
    tap = 0;

    tap = new jtagtap<0x10e31913,
                      7,
                      5>("tap");
    tap->i_trst(i_trst);
    tap->i_tck(i_tck);
    tap->i_tms(i_tms);
    tap->i_tdi(i_tdi);
    tap->o_tdo(o_tdo);
    tap->o_dmi_req_valid(w_tap_dmi_req_valid);
    tap->o_dmi_req_write(w_tap_dmi_req_write);
    tap->o_dmi_req_addr(wb_tap_dmi_req_addr);
    tap->o_dmi_req_data(wb_tap_dmi_req_data);
    tap->i_dmi_resp_data(wb_jtag_dmi_resp_data);
    tap->i_dmi_busy(w_jtag_dmi_busy);
    tap->i_dmi_error(w_jtag_dmi_error);
    tap->o_dmi_hardreset(w_tap_dmi_hardreset);


    cdc = new jtagcdc("cdc", async_reset);
    cdc->i_clk(i_clk);
    cdc->i_nrst(i_nrst);
    cdc->i_dmi_req_valid(w_tap_dmi_req_valid);
    cdc->i_dmi_req_write(w_tap_dmi_req_write);
    cdc->i_dmi_req_addr(wb_tap_dmi_req_addr);
    cdc->i_dmi_req_data(wb_tap_dmi_req_data);
    cdc->i_dmi_hardreset(w_tap_dmi_hardreset);
    cdc->i_dmi_req_ready(w_cdc_dmi_req_ready);
    cdc->o_dmi_req_valid(w_cdc_dmi_req_valid);
    cdc->o_dmi_req_write(w_cdc_dmi_req_write);
    cdc->o_dmi_req_addr(wb_cdc_dmi_req_addr);
    cdc->o_dmi_req_data(wb_cdc_dmi_req_data);
    cdc->o_dmi_hardreset(w_cdc_dmi_hardreset);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_trst;
    sensitive << i_tck;
    sensitive << i_tms;
    sensitive << i_tdi;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << i_halted;
    sensitive << i_available;
    sensitive << i_dport_req_ready;
    sensitive << i_dport_resp_valid;
    sensitive << i_dport_resp_error;
    sensitive << i_dport_rdata;
    sensitive << w_tap_dmi_req_valid;
    sensitive << w_tap_dmi_req_write;
    sensitive << wb_tap_dmi_req_addr;
    sensitive << wb_tap_dmi_req_data;
    sensitive << w_tap_dmi_hardreset;
    sensitive << w_cdc_dmi_req_valid;
    sensitive << w_cdc_dmi_req_ready;
    sensitive << w_cdc_dmi_req_write;
    sensitive << wb_cdc_dmi_req_addr;
    sensitive << wb_cdc_dmi_req_data;
    sensitive << w_cdc_dmi_hardreset;
    sensitive << wb_jtag_dmi_resp_data;
    sensitive << w_jtag_dmi_busy;
    sensitive << w_jtag_dmi_error;
    sensitive << r.bus_jtag;
    sensitive << r.jtag_resp_data;
    sensitive << r.prdata;
    sensitive << r.regidx;
    sensitive << r.wdata;
    sensitive << r.regwr;
    sensitive << r.regrd;
    sensitive << r.dmstate;
    sensitive << r.cmdstate;
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
    sensitive << r.pready;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

dmidebug::~dmidebug() {
    if (cdc) {
        delete cdc;
    }
    if (tap) {
        delete tap;
    }
}

void dmidebug::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_trst, i_trst.name());
        sc_trace(o_vcd, i_tck, i_tck.name());
        sc_trace(o_vcd, i_tms, i_tms.name());
        sc_trace(o_vcd, i_tdi, i_tdi.name());
        sc_trace(o_vcd, o_tdo, o_tdo.name());
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_ndmreset, o_ndmreset.name());
        sc_trace(o_vcd, i_halted, i_halted.name());
        sc_trace(o_vcd, i_available, i_available.name());
        sc_trace(o_vcd, o_hartsel, o_hartsel.name());
        sc_trace(o_vcd, o_haltreq, o_haltreq.name());
        sc_trace(o_vcd, o_resumereq, o_resumereq.name());
        sc_trace(o_vcd, o_resethaltreq, o_resethaltreq.name());
        sc_trace(o_vcd, o_hartreset, o_hartreset.name());
        sc_trace(o_vcd, o_dport_req_valid, o_dport_req_valid.name());
        sc_trace(o_vcd, o_dport_req_type, o_dport_req_type.name());
        sc_trace(o_vcd, o_dport_addr, o_dport_addr.name());
        sc_trace(o_vcd, o_dport_wdata, o_dport_wdata.name());
        sc_trace(o_vcd, o_dport_size, o_dport_size.name());
        sc_trace(o_vcd, i_dport_req_ready, i_dport_req_ready.name());
        sc_trace(o_vcd, o_dport_resp_ready, o_dport_resp_ready.name());
        sc_trace(o_vcd, i_dport_resp_valid, i_dport_resp_valid.name());
        sc_trace(o_vcd, i_dport_resp_error, i_dport_resp_error.name());
        sc_trace(o_vcd, i_dport_rdata, i_dport_rdata.name());
        sc_trace(o_vcd, o_progbuf, o_progbuf.name());
        sc_trace(o_vcd, r.bus_jtag, pn + ".r_bus_jtag");
        sc_trace(o_vcd, r.jtag_resp_data, pn + ".r_jtag_resp_data");
        sc_trace(o_vcd, r.prdata, pn + ".r_prdata");
        sc_trace(o_vcd, r.regidx, pn + ".r_regidx");
        sc_trace(o_vcd, r.wdata, pn + ".r_wdata");
        sc_trace(o_vcd, r.regwr, pn + ".r_regwr");
        sc_trace(o_vcd, r.regrd, pn + ".r_regrd");
        sc_trace(o_vcd, r.dmstate, pn + ".r_dmstate");
        sc_trace(o_vcd, r.cmdstate, pn + ".r_cmdstate");
        sc_trace(o_vcd, r.haltreq, pn + ".r_haltreq");
        sc_trace(o_vcd, r.resumereq, pn + ".r_resumereq");
        sc_trace(o_vcd, r.resumeack, pn + ".r_resumeack");
        sc_trace(o_vcd, r.hartreset, pn + ".r_hartreset");
        sc_trace(o_vcd, r.resethaltreq, pn + ".r_resethaltreq");
        sc_trace(o_vcd, r.ndmreset, pn + ".r_ndmreset");
        sc_trace(o_vcd, r.dmactive, pn + ".r_dmactive");
        sc_trace(o_vcd, r.hartsel, pn + ".r_hartsel");
        sc_trace(o_vcd, r.cmd_regaccess, pn + ".r_cmd_regaccess");
        sc_trace(o_vcd, r.cmd_quickaccess, pn + ".r_cmd_quickaccess");
        sc_trace(o_vcd, r.cmd_memaccess, pn + ".r_cmd_memaccess");
        sc_trace(o_vcd, r.cmd_progexec, pn + ".r_cmd_progexec");
        sc_trace(o_vcd, r.cmd_read, pn + ".r_cmd_read");
        sc_trace(o_vcd, r.cmd_write, pn + ".r_cmd_write");
        sc_trace(o_vcd, r.postincrement, pn + ".r_postincrement");
        sc_trace(o_vcd, r.aamvirtual, pn + ".r_aamvirtual");
        sc_trace(o_vcd, r.command, pn + ".r_command");
        sc_trace(o_vcd, r.autoexecdata, pn + ".r_autoexecdata");
        sc_trace(o_vcd, r.autoexecprogbuf, pn + ".r_autoexecprogbuf");
        sc_trace(o_vcd, r.cmderr, pn + ".r_cmderr");
        sc_trace(o_vcd, r.data0, pn + ".r_data0");
        sc_trace(o_vcd, r.data1, pn + ".r_data1");
        sc_trace(o_vcd, r.data2, pn + ".r_data2");
        sc_trace(o_vcd, r.data3, pn + ".r_data3");
        sc_trace(o_vcd, r.progbuf_data, pn + ".r_progbuf_data");
        sc_trace(o_vcd, r.dport_req_valid, pn + ".r_dport_req_valid");
        sc_trace(o_vcd, r.dport_addr, pn + ".r_dport_addr");
        sc_trace(o_vcd, r.dport_wdata, pn + ".r_dport_wdata");
        sc_trace(o_vcd, r.dport_size, pn + ".r_dport_size");
        sc_trace(o_vcd, r.dport_resp_ready, pn + ".r_dport_resp_ready");
        sc_trace(o_vcd, r.pready, pn + ".r_pready");
    }

    if (cdc) {
        cdc->generateVCD(i_vcd, o_vcd);
    }
    if (tap) {
        tap->generateVCD(i_vcd, o_vcd);
    }
}

void dmidebug::comb() {
    dev_config_type vcfg;
    apb_out_type vapbo;
    sc_uint<DPortReq_Total> vb_req_type;
    sc_uint<32> vb_resp_data;
    sc_uint<CFG_LOG2_CPU_MAX> vb_hartselnext;
    bool v_resp_valid;
    int hsel;
    bool v_cmd_busy;
    bool v_cdc_dmi_req_ready;
    sc_uint<64> vb_arg1;
    sc_uint<32> t_command;
    sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)> t_progbuf;
    int t_idx;

    vcfg = dev_config_none;
    vapbo = apb_out_none;
    vb_req_type = 0;
    vb_resp_data = 0;
    vb_hartselnext = 0;
    v_resp_valid = 0;
    hsel = 0;
    v_cmd_busy = 0;
    v_cdc_dmi_req_ready = 0;
    vb_arg1 = 0;
    t_command = 0;
    t_progbuf = 0;
    t_idx = 0;

    v = r;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.read().addr_start;
    vcfg.addr_end = i_mapinfo.read().addr_end;
    vcfg.vid = VENDOR_OPTIMITECH;
    vcfg.did = OPTIMITECH_RIVER_DMI;

    vb_hartselnext = r.wdata.read()(((16 + CFG_LOG2_CPU_MAX) - 1), 16);
    hsel = r.hartsel.read().to_int();
    v_cmd_busy = r.cmdstate.read().or_reduce();
    vb_arg1 = (r.data3.read(), r.data2.read());
    t_command = r.command;
    t_progbuf = r.progbuf_data;
    t_idx = r.regidx.read()(3, 0).to_int();

    if ((r.haltreq.read() & i_halted.read()[hsel]) == 1) {
        v.haltreq = 0;
    }
    if ((r.resumereq.read() & (~i_halted.read()[hsel])) == 1) {
        v.resumereq = 0;
        v.resumeack = 1;
    }

    switch (r.dmstate.read()) {
    case DM_STATE_IDLE:
        if (w_cdc_dmi_req_valid.read() == 1) {
            v_cdc_dmi_req_ready = 1;
            v.bus_jtag = 1;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = wb_cdc_dmi_req_addr;
            v.wdata = wb_cdc_dmi_req_data;
            v.regwr = w_cdc_dmi_req_write;
            v.regrd = (~w_cdc_dmi_req_write.read());
        } else if (((i_apbi.read().pselx == 1) && (i_apbi.read().pwrite == 0))
                    || ((i_apbi.read().pselx == 1) && (i_apbi.read().penable == 1) && (i_apbi.read().pwrite == 1))) {
            v.bus_jtag = 0;
            v.dmstate = DM_STATE_ACCESS;
            v.regidx = i_apbi.read().paddr(6, 0);
            v.wdata = i_apbi.read().pwdata;
            v.regwr = i_apbi.read().pwrite;
            v.regrd = (!i_apbi.read().pwrite);
        }
        break;
    case DM_STATE_ACCESS:
        v_resp_valid = 1;
        v.dmstate = DM_STATE_IDLE;
        if (r.regidx.read() == 0x04) {                      // arg0[31:0]
            vb_resp_data = r.data0;
            if (r.regwr.read() == 1) {
                v.data0 = r.wdata;
            }
            if ((r.autoexecdata.read()[0] == 1) && (r.cmderr.read() == CMDERR_NONE)) {
                if (v_cmd_busy == 1) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x05) {               // arg0[63:32]
            vb_resp_data = r.data1;
            if (r.regwr.read() == 1) {
                v.data1 = r.wdata;
            }
            if ((r.autoexecdata.read()[1] == 1) && (r.cmderr.read() == CMDERR_NONE)) {
                if (v_cmd_busy == 1) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x06) {               // arg1[31:0]
            vb_resp_data = r.data2;
            if (r.regwr.read() == 1) {
                v.data2 = r.wdata;
            }
            if ((r.autoexecdata.read()[2] == 1) && (r.cmderr.read() == CMDERR_NONE)) {
                if (v_cmd_busy == 1) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x07) {               // arg1[63:32]
            vb_resp_data = r.data3;
            if (r.regwr.read() == 1) {
                v.data3 = r.wdata;
            }
            if ((r.autoexecdata.read()[3] == 1) && (r.cmderr.read() == CMDERR_NONE)) {
                if (v_cmd_busy == 1) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x10) {               // dmcontrol
            vb_resp_data[29] = r.hartreset.read();          // hartreset
            vb_resp_data[28] = 0;                           // ackhavereset
            vb_resp_data[26] = 0;                           // hasel: single selected hart only
            vb_resp_data(((16 + CFG_LOG2_CPU_MAX) - 1), 16) = r.hartsel;// hartsello
            vb_resp_data[1] = r.ndmreset.read();
            vb_resp_data[0] = r.dmactive.read();
            if (r.regwr.read() == 1) {
                if (r.wdata.read()[31] == 1) {
                    // Do not set cmderr before/after ndmreset
                    if (((r.wdata.read()[1] || r.ndmreset) == 0)
                            && (i_halted.read()[vb_hartselnext] == 1)) {
                        v.cmderr = CMDERR_WRONGSTATE;
                    } else {
                        v.haltreq = 1;
                    }
                } else if (r.wdata.read()[30] == 1) {
                    // resumereq must be ignored if haltreq is set (@see spec)
                    if (i_halted.read()[vb_hartselnext] == 1) {
                        v.resumereq = 1;
                    } else {
                        v.cmderr = CMDERR_WRONGSTATE;
                    }
                }
                v.hartreset = r.wdata.read()[29];
                v.hartsel = r.wdata.read()(((16 + CFG_LOG2_CPU_MAX) - 1), 16);
                if (r.wdata.read()[3] == 1) {               // setresethaltreq
                    v.resethaltreq = 1;
                } else if (r.wdata.read()[2] == 1) {        // clearresethaltreq
                    v.resethaltreq = 0;
                }
                v.ndmreset = r.wdata.read()[1];             // 1=Reset whole system including all harts
                v.dmactive = r.wdata.read()[0];
            }
        } else if (r.regidx.read() == 0x11) {               // dmstatus
            // Currently selected ONLY. We support only one selected at once 'hasel=0'
            vb_resp_data[22] = 0;                           // impebreak
            vb_resp_data[19] = 0;                           // allhavereset: selected hart reset but not acknowledged
            vb_resp_data[18] = 0;                           // anyhavereset
            vb_resp_data[17] = r.resumeack.read();          // allresumeack
            vb_resp_data[16] = r.resumeack.read();          // anyresumeack
            vb_resp_data[15] = (!i_available.read()[hsel]); // allnonexistent
            vb_resp_data[14] = (!i_available.read()[hsel]); // anynonexistent
            vb_resp_data[13] = (!i_available.read()[hsel]); // allunavail
            vb_resp_data[12] = (!i_available.read()[hsel]); // anyunavail
            vb_resp_data[11] = ((!i_halted.read()[hsel]) && i_available.read()[hsel]);// allrunning:
            vb_resp_data[10] = ((!i_halted.read()[hsel]) && i_available.read()[hsel]);// anyrunning:
            vb_resp_data[9] = (i_halted.read()[hsel] && i_available.read()[hsel]);// allhalted:
            vb_resp_data[8] = (i_halted.read()[hsel] && i_available.read()[hsel]);// anyhalted:
            vb_resp_data[7] = 1;                            // authenticated:
            vb_resp_data[5] = 1;                            // hasresethaltreq
            vb_resp_data(3, 0) = 2;                         // version: dbg spec v0.13
        } else if (r.regidx.read() == 0x12) {               // hartinfo
            // Not available core should returns 0
            if (i_available.read()[hsel] == 1) {
                vb_resp_data(23, 20) = CFG_DSCRATCH_REG_TOTAL;// nscratch
                vb_resp_data[16] = 0;                       // dataaccess: 0=CSR shadowed;1=memory shadowed
                vb_resp_data(15, 12) = 0;                   // datasize
                vb_resp_data(11, 0) = 0;                    // dataaddr
            }
        } else if (r.regidx.read() == 0x16) {               // abstractcs
            vb_resp_data(28, 24) = CFG_PROGBUF_REG_TOTAL;
            vb_resp_data[12] = v_cmd_busy;                  // busy
            vb_resp_data(10, 8) = r.cmderr;
            vb_resp_data(3, 0) = CFG_DATA_REG_TOTAL;
            if ((r.regwr.read() == 1) && (r.wdata.read()(10, 8) == 1)) {
                v.cmderr = CMDERR_NONE;
            }
        } else if (r.regidx.read() == 0x17) {               // command
            if (r.regwr.read() == 1) {
                if (r.cmderr.read() == CMDERR_NONE) {
                    // If cmderr is non-zero, writes to this register are ignores (@see spec)
                    if (v_cmd_busy == 1) {
                        v.cmderr = CMDERR_BUSY;
                    } else {
                        v.command = r.wdata;
                        v.cmdstate = CMD_STATE_INIT;
                    }
                }
            }
        } else if (r.regidx.read() == 0x18) {               // abstractauto
            vb_resp_data((CFG_DATA_REG_TOTAL - 1), 0) = r.autoexecdata;
            vb_resp_data(((16 + CFG_PROGBUF_REG_TOTAL) - 1), 16) = r.autoexecprogbuf;
            if (r.regwr.read() == 1) {
                v.autoexecdata = r.wdata.read()((CFG_DATA_REG_TOTAL - 1), 0);
                v.autoexecprogbuf = r.wdata.read()(((16 + CFG_PROGBUF_REG_TOTAL) - 1), 16);
            }
        } else if (r.regidx.read()(6, 4) == 0x02) {         // progbuf[n]
            vb_resp_data = r.progbuf_data.read()((32 * t_idx) + 32 - 1, (32 * t_idx)).to_uint64();
            if (r.regwr.read() == 1) {
                t_progbuf((32 * t_idx) + 32- 1, (32 * t_idx)) = r.wdata;
                v.progbuf_data = t_progbuf;
            }
            if ((r.autoexecprogbuf.read()[t_idx] == 1)
                    && (r.cmderr.read() == CMDERR_NONE)) {
                if (v_cmd_busy == 1) {
                    v.cmderr = CMDERR_BUSY;
                } else {
                    v.cmdstate = CMD_STATE_INIT;
                }
            }
        } else if (r.regidx.read() == 0x40) {               // haltsum0
            vb_resp_data((CFG_CPU_MAX - 1), 0) = i_halted;
        }
        break;
    default:
        break;
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
        if (r.command.read()(31, 24) == 0) {
            // Register access command
            if (r.command.read()[CmdTransferBit] == 1) {    // transfer
                v.cmdstate = CMD_STATE_REQUEST;
                v.cmd_regaccess = 1;
                v.cmd_read = (!r.command.read()[CmdWriteBit]);
                v.cmd_write = r.command.read()[CmdWriteBit];

                v.dport_req_valid = 1;
                v.dport_addr = (0ull, r.command.read()(15, 0));
                v.dport_wdata = (r.data1.read(), r.data0.read());
                v.dport_size = r.command.read()(22, 20);
            } else if (r.command.read()[CmdPostexecBit] == 1) {// no transfer only buffer execution
                v.cmdstate = CMD_STATE_REQUEST;

                v.dport_req_valid = 1;
                v.cmd_progexec = 1;
            } else {
                // Empty command: do nothing
                v.cmdstate = CMD_STATE_IDLE;
            }
        } else if (r.command.read()(31, 24) == 1) {
            // Quick access command
            if (i_halted.read()[hsel] == 1) {
                v.cmderr = CMDERR_WRONGSTATE;
                v.cmdstate = CMD_STATE_IDLE;
            } else {
                v.haltreq = 1;
                v.cmd_quickaccess = 1;
                v.cmdstate = CMD_STATE_WAIT_HALTED;
            }
        } else if (r.command.read()(31, 24) == 2) {
            // Memory access command
            v.cmdstate = CMD_STATE_REQUEST;
            v.cmd_memaccess = 1;
            v.cmd_read = (!r.command.read()[CmdWriteBit]);
            v.cmd_write = r.command.read()[CmdWriteBit];
            v.aamvirtual = r.command.read()[23];

            v.dport_req_valid = 1;
            v.dport_addr = (r.data3.read(), r.data2.read());
            v.dport_wdata = (r.data1.read(), r.data0.read());
            v.dport_size = r.command.read()(22, 20);
        } else {
            // Unsupported command type:
            v.cmdstate = CMD_STATE_IDLE;
        }
        break;
    case CMD_STATE_REQUEST:
        if (i_dport_req_ready.read() == 1) {
            v.cmdstate = CMD_STATE_RESPONSE;
            v.dport_req_valid = 0;
            v.dport_resp_ready = 1;
        }
        break;
    case CMD_STATE_RESPONSE:
        if (i_dport_resp_valid.read() == 1) {
            v.dport_resp_ready = 0;
            if (r.cmd_read.read() == 1) {
                switch (r.command.read()(22, 20)) {         // aamsize/aarsize
                case 0:
                    v.data0 = (0, i_dport_rdata.read()(7, 0));
                    v.data1 = 0;
                    break;
                case 1:
                    v.data0 = (0, i_dport_rdata.read()(15, 0));
                    v.data1 = 0;
                    break;
                case 2:
                    v.data0 = i_dport_rdata.read()(31, 0);
                    v.data1 = 0;
                    break;
                case 3:
                    v.data0 = i_dport_rdata.read()(31, 0);
                    v.data1 = i_dport_rdata.read()(63, 32);
                    break;
                default:
                    break;
                }
            }
            if (r.postincrement.read() == 1) {
                v.postincrement = 0;
                if (r.command.read()(31, 24) == 0) {
                    // Register access command:
                    t_command(15, 0) = (r.command.read()(15, 0) + 1);
                    v.command = t_command;
                } else if (r.command.read()(31, 24) == 2) {
                    // Memory access command
                    switch (r.command.read()(22, 20)) {     // aamsize
                    case 0:
                        vb_arg1 = (vb_arg1 + 1ull);
                        break;
                    case 1:
                        vb_arg1 = (vb_arg1 + 2ull);
                        break;
                    case 2:
                        vb_arg1 = (vb_arg1 + 4ull);
                        break;
                    case 3:
                        vb_arg1 = (vb_arg1 + 8ull);
                        break;
                    default:
                        break;
                    }
                    v.data3 = vb_arg1(63, 32);
                    v.data2 = vb_arg1(31, 0);
                }
            }
            if (i_dport_resp_error.read() == 1) {
                v.cmdstate = CMD_STATE_IDLE;
                if (r.cmd_memaccess.read() == 1) {
                    // @spec The abstract command failed due to a bus error
                    //       (e.g. alignment, access size, or timeout).
                    v.cmderr = CMDERR_BUSERROR;
                } else {
                    // @spec  An exception occurred while executing the
                    //        command (e.g. while executing the Program Buffer).
                    v.cmderr = CMDERR_EXCEPTION;
                }
            } else if ((r.cmd_regaccess.read() == 1) && (r.command.read()[CmdPostexecBit] == 1)) {
                v.dport_req_valid = 1;
                v.cmd_progexec = 1;
                v.cmd_regaccess = 0;
                v.cmd_write = 0;
                v.cmd_read = 0;
                v.cmdstate = CMD_STATE_REQUEST;
            } else {
                v.cmdstate = CMD_STATE_IDLE;
            }
            if (r.cmd_quickaccess.read() == 1) {
                // fast command continued even if progbuf execution failed (@see spec)
                v.resumereq = 1;
            }
        }
        break;
    case CMD_STATE_WAIT_HALTED:
        if (i_halted.read()[hsel] == 1) {
            v.cmdstate = CMD_STATE_REQUEST;

            v.dport_req_valid = 1;
            v.cmd_progexec = 1;
        }
        break;
    default:
        break;
    }

    if (v_resp_valid == 1) {
        if (r.bus_jtag.read() == 0) {
            v.prdata = vb_resp_data;
        } else {
            v.jtag_resp_data = vb_resp_data;
        }
    }
    v.pready = 0;
    if ((v_resp_valid == 1) && (r.bus_jtag.read() == 0)) {
        v.pready = 1;
    } else if (i_apbi.read().penable == 1) {
        v.pready = 0;
    }

    vapbo.pready = r.pready;
    vapbo.prdata = r.prdata;

    vb_req_type[DPortReq_Write] = r.cmd_write.read();
    vb_req_type[DPortReq_RegAccess] = r.cmd_regaccess.read();
    vb_req_type[DPortReq_MemAccess] = r.cmd_memaccess.read();
    vb_req_type[DPortReq_MemVirtual] = r.aamvirtual.read();
    vb_req_type[DPortReq_Progexec] = r.cmd_progexec.read();

    if ((!async_reset_ && i_nrst.read() == 0) || (w_cdc_dmi_hardreset.read() == 1)) {
        dmidebug_r_reset(v);
    }

    o_ndmreset = r.ndmreset;
    o_hartsel = r.hartsel;
    o_haltreq = r.haltreq;
    o_resumereq = r.resumereq;
    o_resethaltreq = r.resethaltreq;
    o_hartreset = r.hartreset;
    o_dport_req_valid = r.dport_req_valid;
    o_dport_req_type = vb_req_type;
    o_dport_addr = r.dport_addr;
    o_dport_wdata = r.dport_wdata;
    o_dport_size = r.dport_size;
    o_dport_resp_ready = r.dport_resp_ready;
    o_progbuf = r.progbuf_data;

    w_cdc_dmi_req_ready = v_cdc_dmi_req_ready;
    wb_jtag_dmi_resp_data = r.jtag_resp_data;
    w_jtag_dmi_busy = r.dmstate;
    // There are no specified cases in which the DM would respond with an error,
    // and DMI is not required to support returning errors.
    w_jtag_dmi_error = 0;

    o_cfg = vcfg;
    o_apbo = vapbo;
}

void dmidebug::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        dmidebug_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

