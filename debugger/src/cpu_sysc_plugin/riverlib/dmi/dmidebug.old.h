/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include "api_core.h"
#include "iservice.h"
#include "riverlib/river_cfg.h"
#include "riverlib/types_river.h"
#include "ambalib/types_amba.h"
#include "jtagtap.h"
#include "jtagcdc.h"
#include <systemc.h>

namespace debugger {

class DmiDebug : public sc_module {

 public:
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // JTAG interface
    sc_in<bool> i_trst;
    sc_in<bool> i_tck;
    sc_in<bool> i_tms;
    sc_in<bool> i_tdi;
    sc_out<bool> o_tdo;
    // Bus interface (APB)
    sc_in<bool> i_bus_req_valid;
    sc_out<bool> o_bus_req_ready;
    sc_in<sc_uint<7>> i_bus_req_addr;
    sc_in<bool> i_bus_req_write;
    sc_in<sc_uint<32>> i_bus_req_wdata;
    sc_out<bool> o_bus_resp_valid;
    sc_in<bool> i_bus_resp_ready;
    sc_out<sc_uint<32>> o_bus_resp_rdata;
    // DMI interface
    sc_out<bool> o_ndmreset;                            // whole system reset including all cores
    sc_in<sc_uint<CFG_CPU_MAX>> i_halted;               // Halted cores
    sc_in<sc_uint<CFG_CPU_MAX>> i_available;            // Existing and available cores
    sc_out<sc_uint<CFG_LOG2_CPU_MAX>> o_hartsel;        // Selected hart index
    sc_in<dport_out_type> i_dporto;
    sc_out<dport_in_type> o_dporti;
    sc_out<sc_biguint<CFG_PROGBUF_REG_TOTAL*32>> o_progbuf;

    void comb();
    void registers();

    SC_HAS_PROCESS(DmiDebug);

    DmiDebug(IFace *parent, sc_module_name name, bool async_reset);
    virtual ~DmiDebug();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    IFace *iparent_;    // pointer on parent module object (used for logging)

    JtagTap *tap_;
    JtagCDC *cdc_;

    static const unsigned DM_STATE_IDLE = 0;
    static const unsigned DM_STATE_ACCESS = 1;

    static const unsigned CMD_STATE_IDLE    = 0;
    static const unsigned CMD_STATE_INIT    = 1;
    static const unsigned CMD_STATE_REQUEST = 2;
    static const unsigned CMD_STATE_RESPONSE = 3;
    static const unsigned CMD_STATE_WAIT_HALTED = 4;

    static const unsigned CMDERR_NONE = 0;
    static const unsigned CMDERR_BUSY = 1;
    static const unsigned CMDERR_NOTSUPPROTED = 2;
    static const unsigned CMDERR_EXCEPTION = 3;
    static const unsigned CMDERR_WRONGSTATE = 4;
    static const unsigned CMDERR_BUSERROR = 5;
    static const unsigned CMDERR_OTHERS = 7;

    static const int CmdPostexecBit = 18;
    static const int CmdTransferBit = 17;
    static const int CmdWriteBit = 16;
    static const int CmdPostincrementBit = 19;

    sc_signal<bool> w_tap_dmi_req_valid;
    sc_signal<bool> w_tap_dmi_req_write;
    sc_signal<sc_uint<7>> wb_tap_dmi_req_addr;
    sc_signal<sc_uint<32>> wb_tap_dmi_req_data;
    sc_signal<bool> w_tap_dmi_reset;
    sc_signal<bool> w_tap_dmi_hardreset;

    sc_signal<bool> w_cdc_dmi_req_valid;
    sc_signal<bool> w_cdc_dmi_req_ready;
    sc_signal<bool> w_cdc_dmi_req_write;
    sc_signal<sc_uint<7>> wb_cdc_dmi_req_addr;
    sc_signal<sc_uint<32>> wb_cdc_dmi_req_data;
    sc_signal<bool> w_cdc_dmi_reset;
    sc_signal<bool> w_cdc_dmi_hardreset;
    
    sc_signal<sc_uint<32>> wb_jtag_dmi_resp_data;
    sc_signal<bool> w_jtag_dmi_busy;
    sc_signal<bool> w_jtag_dmi_error;

    sc_event bus_req_event_;
    sc_event bus_resp_event_;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_wdata;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> wb_wstrb;
    sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> wb_resp_data;

    struct RegistersType {
        sc_signal<bool> bus_jtag;       // source of the DMI request

        sc_signal<bool> bus_resp_valid;
        sc_signal<sc_uint<32>> bus_resp_data;
        sc_signal<sc_uint<32>> jtag_resp_data;

        sc_signal<sc_uint<7>> regidx;
        sc_signal<sc_uint<32>> wdata;
        sc_signal<bool> regwr;
        sc_signal<bool> regrd;

        sc_signal<sc_uint<1>> dmstate;
        sc_signal<sc_uint<3>> cmdstate;
        sc_signal<sc_uint<3>> sbastate;

        sc_signal<bool> haltreq;
        sc_signal<bool> resumereq;
        sc_signal<bool> resumeack;
        sc_signal<bool> hartreset;
        sc_signal<bool> resethaltreq;   // halt on reset
        sc_signal<bool> ndmreset;
        sc_signal<bool> dmactive;
        sc_signal<sc_uint<CFG_LOG2_CPU_MAX>> hartsel;
        sc_signal<bool> cmd_regaccess;
        sc_signal<bool> cmd_quickaccess;
        sc_signal<bool> cmd_memaccess;
        sc_signal<bool> cmd_progexec;
        sc_signal<bool> cmd_read;
        sc_signal<bool> cmd_write;
        sc_signal<bool> postincrement;
        sc_signal<bool> aamvirtual;
        sc_signal<sc_uint<32>> command;
        sc_signal<sc_uint<CFG_DATA_REG_TOTAL>> autoexecdata;
        sc_signal<sc_uint<CFG_PROGBUF_REG_TOTAL>> autoexecprogbuf;
        sc_signal<sc_uint<3>> cmderr;
        sc_signal<sc_uint<32>> data0;
        sc_signal<sc_uint<32>> data1;
        sc_signal<sc_uint<32>> data2;
        sc_signal<sc_uint<32>> data3;
        sc_signal<sc_biguint<CFG_PROGBUF_REG_TOTAL*32>> progbuf_data;

        sc_signal<bool> dport_req_valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> dport_addr;
        sc_signal<sc_uint<RISCV_ARCH>> dport_wdata;
        sc_signal<sc_uint<3>> dport_size;
        sc_signal<bool> dport_resp_ready;
    } r, v;

    void R_RESET(RegistersType &iv) {
        iv.bus_jtag = 0;
        iv.bus_resp_valid = 0;
        iv.bus_resp_data = 0;

        iv.regidx = 0;
        iv.wdata = 0;
        iv.regwr = 0;
        iv.regrd = 0;
        iv.dmstate = DM_STATE_IDLE;
        iv.cmdstate = CMD_STATE_IDLE;
        iv.sbastate = 0;

        iv.haltreq = 0;
        iv.resumereq = 0;
        iv.resumeack = 0;
        iv.hartreset = 0;
        iv.resethaltreq = 0;
        iv.ndmreset = 0;
        iv.dmactive = 0;
        iv.hartsel = 0;
        iv.cmd_regaccess = 0;
        iv.cmd_quickaccess = 0;
        iv.cmd_memaccess = 0;
        iv.cmd_progexec = 0;
        iv.cmd_write = 0;
        iv.cmd_read = 0;
        iv.postincrement = 0;
        iv.aamvirtual = 0;
        iv.command = 0;
        iv.autoexecdata = 0;
        iv.autoexecprogbuf = 0;
        iv.cmderr = 0;
        iv.data0 = 0;
        iv.data1 = 0;
        iv.data2 = 0;
        iv.data3 = 0;
        iv.progbuf_data = 0;
        iv.dport_req_valid = 0;
        iv.dport_addr = 0;
        iv.dport_wdata = 0;
        iv.dport_size = 0;
        iv.dport_resp_ready = 0;
    }
    bool async_reset_;
};

}  // namespace debugger

