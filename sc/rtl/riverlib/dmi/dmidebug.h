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
#pragma once

#include <systemc.h>
#include "../../ambalib/types_amba.h"
#include "../river_cfg.h"
#include "jtagcdc.h"
#include "jtagtap.h"

namespace debugger {

SC_MODULE(dmidebug) {
 public:
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;                                     // full reset including dmi (usually via reset button)
    // JTAG interface:
    sc_in<bool> i_trst;                                     // Test reset: must be open-drain, pullup
    sc_in<bool> i_tck;                                      // Test Clock
    sc_in<bool> i_tms;                                      // Test Mode State
    sc_in<bool> i_tdi;                                      // Test Data Input
    sc_out<bool> o_tdo;                                     // Test Data Output
    // Bus interface (APB):
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB input interface
    sc_out<apb_out_type> o_apbo;                            // APB output interface
    // DMI interface:
    sc_out<bool> o_ndmreset;                                // system reset: cores + peripheries (except dmi itself)
    sc_in<sc_uint<CFG_CPU_MAX>> i_halted;                   // Halted cores
    sc_in<sc_uint<CFG_CPU_MAX>> i_available;                // Existing and available cores
    sc_out<sc_uint<CFG_LOG2_CPU_MAX>> o_hartsel;            // Selected hart index
    sc_out<bool> o_haltreq;
    sc_out<bool> o_resumereq;
    sc_out<bool> o_resethaltreq;                            // Halt core after reset request.
    sc_out<bool> o_hartreset;                               // Reset currently selected hart
    sc_out<bool> o_dport_req_valid;                         // Debug access from DSU is valid
    sc_out<sc_uint<DPortReq_Total>> o_dport_req_type;       // Debug access types
    sc_out<sc_uint<RISCV_ARCH>> o_dport_addr;               // Register index
    sc_out<sc_uint<RISCV_ARCH>> o_dport_wdata;              // Write value
    sc_out<sc_uint<3>> o_dport_size;                        // 0=1B;1=2B;2=4B;3=8B;4=128B
    sc_in<bool> i_dport_req_ready;                          // Response is ready
    sc_out<bool> o_dport_resp_ready;                        // ready to accepd response
    sc_in<bool> i_dport_resp_valid;                         // Response is valid
    sc_in<bool> i_dport_resp_error;                         // Something goes wrong
    sc_in<sc_uint<RISCV_ARCH>> i_dport_rdata;               // Response value or error code
    sc_out<sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)>> o_progbuf;

    void comb();
    void registers();

    SC_HAS_PROCESS(dmidebug);

    dmidebug(sc_module_name name,
             bool async_reset);
    virtual ~dmidebug();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t CMDERR_NONE = 0;
    static const uint8_t CMDERR_BUSY = 1;
    static const uint8_t CMDERR_NOTSUPPROTED = 2;
    static const uint8_t CMDERR_EXCEPTION = 3;
    static const uint8_t CMDERR_WRONGSTATE = 4;
    static const uint8_t CMDERR_BUSERROR = 5;
    static const uint8_t CMDERR_OTHERS = 7;
    // Dedicated bit in the 'command' register
    static const int CmdPostexecBit = 18;
    static const int CmdTransferBit = 17;
    static const int CmdWriteBit = 16;
    static const int CmdPostincrementBit = 19;
    // dmstate:
    static const bool DM_STATE_IDLE = 0;
    static const bool DM_STATE_ACCESS = 1;
    // cmdstate:
    static const uint8_t CMD_STATE_IDLE = 0;
    static const uint8_t CMD_STATE_INIT = 1;
    static const uint8_t CMD_STATE_REQUEST = 2;
    static const uint8_t CMD_STATE_RESPONSE = 3;
    static const uint8_t CMD_STATE_WAIT_HALTED = 4;

    struct dmidebug_registers {
        sc_signal<bool> bus_jtag;
        sc_signal<sc_uint<32>> jtag_resp_data;
        sc_signal<sc_uint<32>> prdata;
        sc_signal<sc_uint<7>> regidx;
        sc_signal<sc_uint<32>> wdata;
        sc_signal<bool> regwr;
        sc_signal<bool> regrd;
        sc_signal<bool> dmstate;
        sc_signal<sc_uint<3>> cmdstate;
        sc_signal<bool> haltreq;
        sc_signal<bool> resumereq;
        sc_signal<bool> resumeack;
        sc_signal<bool> hartreset;
        sc_signal<bool> resethaltreq;                       // halt on reset
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
        sc_signal<sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)>> progbuf_data;
        sc_signal<bool> dport_req_valid;
        sc_signal<sc_uint<RISCV_ARCH>> dport_addr;
        sc_signal<sc_uint<RISCV_ARCH>> dport_wdata;
        sc_signal<sc_uint<3>> dport_size;
        sc_signal<bool> dport_resp_ready;
        sc_signal<bool> pready;
    } v, r;

    void dmidebug_r_reset(dmidebug_registers &iv) {
        iv.bus_jtag = 0;
        iv.jtag_resp_data = 0;
        iv.prdata = 0;
        iv.regidx = 0;
        iv.wdata = 0;
        iv.regwr = 0;
        iv.regrd = 0;
        iv.dmstate = DM_STATE_IDLE;
        iv.cmdstate = CMD_STATE_IDLE;
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
        iv.cmd_read = 0;
        iv.cmd_write = 0;
        iv.postincrement = 0;
        iv.aamvirtual = 0;
        iv.command = 0;
        iv.autoexecdata = 0;
        iv.autoexecprogbuf = 0;
        iv.cmderr = CMDERR_NONE;
        iv.data0 = 0;
        iv.data1 = 0;
        iv.data2 = 0;
        iv.data3 = 0;
        iv.progbuf_data = 0ull;
        iv.dport_req_valid = 0;
        iv.dport_addr = 0ull;
        iv.dport_wdata = 0ull;
        iv.dport_size = 0;
        iv.dport_resp_ready = 0;
        iv.pready = 0;
    }

    sc_signal<bool> w_tap_dmi_req_valid;
    sc_signal<bool> w_tap_dmi_req_write;
    sc_signal<sc_uint<7>> wb_tap_dmi_req_addr;
    sc_signal<sc_uint<32>> wb_tap_dmi_req_data;
    sc_signal<bool> w_tap_dmi_hardreset;
    sc_signal<bool> w_cdc_dmi_req_valid;
    sc_signal<bool> w_cdc_dmi_req_ready;
    sc_signal<bool> w_cdc_dmi_req_write;
    sc_signal<sc_uint<7>> wb_cdc_dmi_req_addr;
    sc_signal<sc_uint<32>> wb_cdc_dmi_req_data;
    sc_signal<bool> w_cdc_dmi_hardreset;
    sc_signal<sc_uint<32>> wb_jtag_dmi_resp_data;
    sc_signal<bool> w_jtag_dmi_busy;
    sc_signal<bool> w_jtag_dmi_error;

    jtagcdc *cdc;
    jtagtap<0x10e31913, 7, 5> *tap;

};

}  // namespace debugger

