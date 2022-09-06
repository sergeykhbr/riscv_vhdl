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
#include "coreservices/imemop.h"
#include "coreservices/ireset.h"
#include "coreservices/ijtagtap.h"
#include "riverlib/river_cfg.h"
#include "ambalib/types_amba.h"
#include "jtagtap.h"
#include "jtagcdc.h"
#include <systemc.h>

namespace debugger {

class DmiDebug : public sc_module,
                 public IMemoryOperation,
                 public IJtagTap {

 public:
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_out<bool> o_ndmreset;                            // whole system reset including all cores
    sc_in<sc_uint<CFG_CPU_MAX>> i_halted;               // Halted cores
    sc_in<sc_uint<CFG_CPU_MAX>> i_available;            // Existing and available cores
    sc_out<sc_uint<CFG_LOG2_CPU_MAX>> o_hartsel;        // Selected hart index
    sc_out<bool> o_haltreq;
    sc_out<bool> o_resumereq;
    sc_out<bool> o_resethaltreq;                        // Halt core after reset request.
    sc_out<bool> o_hartreset;                           // Reset currently selected hart
    sc_out<bool> o_dport_req_valid;                     // Debug access from DSU is valid
    sc_out<sc_uint<DPortReq_Total>> o_dport_req_type;   // Debug access types
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_dport_addr;    // Register index
    sc_out<sc_uint<RISCV_ARCH>> o_dport_wdata;          // Write value
    sc_out<sc_uint<3>> o_dport_size;                    // 0=1B;1=2B;2=4B;3=8B;4=128B
    sc_in<bool> i_dport_req_ready;                      // Response is ready
    sc_out<bool> o_dport_resp_ready;                    // ready to accepd response
    sc_in<bool> i_dport_resp_valid;                     // Response is valid
    sc_in<bool> i_dport_resp_error;                     // Something goes wrong
    sc_in<sc_uint<RISCV_ARCH>> i_dport_rdata;           // Response value or error code
    sc_out<sc_biguint<CFG_PROGBUF_REG_TOTAL*32>> o_progbuf;

    void comb();
    void registers();

    SC_HAS_PROCESS(DmiDebug);

    DmiDebug(IFace *parent, sc_module_name name, bool async_reset);
    virtual ~DmiDebug();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                      IAxi4NbResponse *cb) override ;

    /** IJtagTap */
    virtual void resetTAP(char trst, char srst);
    virtual void setPins(char tck, char tms, char tdi);
    virtual bool getTDO();


    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    IFace *getInterface(const char *name) { return iparent_; }

    void readreg(uint64_t idx);
    void writereg(uint64_t idx, uint32_t w32);

 private:
    IFace *iparent_;    // pointer on parent module object (used for logging)

    JtagTap *tap_;
    JtagCDC *cdc_;

    // Last processing transaction
    Axi4TransactionType lasttrans_;
    IAxi4NbResponse *lastcb_;

    bool bus_req_valid_;
    uint32_t bus_req_addr_;
    bool bus_req_write_;
    uint32_t bus_req_wdata_;
    uint32_t bus_resp_data_;
    event_def event_dtm_ready_;

    char trst_;
    char tck_;
    char tms_;
    char tdi_;
    int dtm_scaler_cnt_;

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

    sc_signal<bool> w_i_trst;
    sc_signal<bool> w_i_tck;
    sc_signal<bool> w_i_tms;
    sc_signal<bool> w_i_tdi;
    sc_signal<bool> w_o_tdo;
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
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_wdata;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_wstrb;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_resp_data;

    struct RegistersType {
        sc_signal<bool> bus_jtag;       // source of the DMI request

        sc_signal<bool> bus_req_valid;
        sc_signal<sc_uint<7>> bus_req_addr;
        sc_signal<bool> bus_req_write;
        sc_signal<sc_uint<32>> bus_req_wdata;
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
        iv.bus_req_valid = 0;
        iv.bus_req_addr = 0;
        iv.bus_req_write = 0;
        iv.bus_req_wdata = 0;
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


    union g {
        struct DmiRegsType {
            uint32_t rsrv_00_03[4];     //
            uint32_t data[12];          // 0x04 Abstract Data
            uint32_t dmcontrol;         // 0x10 Debug Module Control
            uint32_t dmstatus;          // 0x11 Debug Module Status
            uint32_t hartinfo;          // 0x12 Hart Info
            uint32_t haltsum1;          // 0x13 Halt Summary 1
            uint32_t hawindowsel;       // 0x14 Hart Array Window Select
            uint32_t hawindow;          // 0x15 Hart Array Window
            uint32_t abstractcs;        // 0x16 Abstract Control and Status
            uint32_t command;           // 0x17 Abstract Command
            uint32_t abstractauto;      // 0x18 Abstract Command Autoexec
            uint32_t confstrptr[4];     // 0x19-0x1c Configuration String Pointers 1-3
            uint32_t nextdm;            // 0x1d Next Debug Module
            uint32_t rsrv_1e_1f[2];
            uint32_t progbuf[16];       // 0x20-0x2f Program buffer 0-15
            uint32_t authdata;          // 0x30 Authentication Data
            uint32_t rsrv_31_33[3];
            uint32_t haltsum2;          // 0x34 Halt summary 2
            uint32_t haltsum3;          // 0x35 Halt summary 3
            uint32_t rsrv_36;
            uint32_t sbaddress3;        // 0x37 System Bus Address [127:96]
            uint32_t sbcs;              // 0x38 System Bus Access Control and Status
            uint32_t sbaddress0;        // 0x39 System Bus Address [31:0]
            uint32_t sbaddress1;        // 0x3a System Bus Address [63:32]
            uint32_t sbaddress2;        // 0x3b System Bus Address [95:64]
            uint32_t sbdata0;           // 0x3a System Bus Data [31:0]
            uint32_t sbdata1;           // 0x3a System Bus Data [63:32]
            uint32_t sbdata2;           // 0x3a System Bus Data [95:64]
            uint32_t sbdata3;           // 0x3a System Bus Data [127:96]
            uint32_t haltsum0;          // 0x40 Halt Summary 0
        } r;
        uint32_t b8[sizeof(DmiRegsType)];
        uint32_t b32[sizeof(DmiRegsType) / sizeof(uint32_t)];
    };

    bool async_reset_;
};

}  // namespace debugger

