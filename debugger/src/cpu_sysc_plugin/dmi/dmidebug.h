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
#include "riverlib/river_cfg.h"
#include <systemc.h>

namespace debugger {

union DmiRegBankType {
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

class DmiDebug : public sc_module,
                 public IMemoryOperation {

 public:
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_out<bool> o_haltreq;
    sc_out<bool> o_resumereq;
    sc_in<bool> i_halted;

    DmiRegBankType bank_;

    static const uint8_t BANK_REG_READ = 1 << 0;
    static const uint8_t BANK_REG_WRITE = 1 << 1;
    uint8_t bankaccess_[sizeof(DmiRegBankType::DmiRegsType) / sizeof(uint32_t)];

    struct RegistersType {
        sc_signal<sc_uint<7>> regidx;
        sc_signal<sc_uint<32>> wdata;
        sc_signal<bool> regwr;
        sc_signal<bool> regrd;

        sc_signal<bool> haltreq;
        sc_signal<bool> resumereq;
        sc_signal<sc_uint<32>> data0;
        sc_signal<sc_uint<32>> data1;
    } r, v;

    sc_event bus_req_event_;
    sc_event bus_resp_event_;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_wdata;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_wstrb;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_resp_data;
    sc_signal<bool> w_r_error;
    sc_signal<bool> w_w_error;

    sc_signal<bool> w_dport_req_valid;
    sc_signal<bool> w_dport_write;
    sc_signal<bool> w_dport_resp_ready;
    sc_signal<sc_uint<CFG_DPORT_ADDR_BITS>> wb_dport_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dport_wdata;


    void comb();
    void registers();

    SC_HAS_PROCESS(DmiDebug);

    DmiDebug(IFace *parent, sc_module_name name);
    virtual ~DmiDebug();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

 public:
    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    IFace *getInterface(const char *name) { return iparent_; }

    void readreg(uint64_t idx, uint8_t *buf);
    void writereg(uint64_t idx, uint8_t *buf);

 private:
    IFace *iparent_;    // pointer on parent module object (used for logging)

    void R_RESET(RegistersType &iv) {
        iv.regidx = 0;
        iv.wdata = 0;
        iv.regwr = 0;
        iv.regrd = 0;

        iv.haltreq = 0;
        iv.resumereq = 0;
        iv.data0 = 0;
        iv.data1 = 0;
    }

    bool async_reset_;

};

}  // namespace debugger

