/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2DESTINATION_H__
#define __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2DESTINATION_H__

#include "api_core.h"
#include "../types_river.h"
#include "../../ambalib/types_amba.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(L2Destination) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_resp_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_resp_rdata;
    sc_in<sc_uint<2>> i_resp_status;
    sc_in<axi4_l1_out_type> i_l1o0;
    sc_out<axi4_l1_in_type> o_l1i0;
    sc_in<axi4_l1_out_type> i_l1o1;
    sc_out<axi4_l1_in_type> o_l1i1;
    sc_in<axi4_l1_out_type> i_l1o2;
    sc_out<axi4_l1_in_type> o_l1i2;
    sc_in<axi4_l1_out_type> i_l1o3;
    sc_out<axi4_l1_in_type> o_l1i3;
    sc_in<axi4_l1_out_type> i_acpo;
    sc_out<axi4_l1_in_type> o_acpi;
    // cache interface
    sc_in<bool> i_req_ready;
    sc_out<bool> o_req_valid;
    sc_out<sc_uint<L2_REQ_TYPE_BITS>> o_req_type;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_addr;
    sc_out<sc_uint<3>> o_req_size;
    sc_out<sc_uint<3>> o_req_prot;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_wdata;
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_wstrb;

    void comb();
    void registers();

    SC_HAS_PROCESS(L2Destination);

    L2Destination(sc_module_name name, bool async_reset);
    virtual ~L2Destination();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    enum EState {
        Idle,
        CacheReadReq,
        CacheWriteReq,
        ReadMem,
        WriteMem,
    };

    enum ETYPE_AR {
        AR_ReadNoSnoop,
        AR_ReadOnce,
        AR_ReadShared,
        AR_ReadClean,
        AR_ReadNotSharedDirty,
        AR_ReadUnique,
        AR_CleanUnique,
        AR_MakeUnique,
        AR_CleanShared,
        AR_CleanInvalid,
        AR_MakeInvalid,
        AR_Barrier,
        AR_DVM_Complete,
        AR_DVM_Message
    };

    enum ETYPE_AW {
        AW_WriteNoSnoop,
        AW_WriteUnique,
        AW_WriteLineUnique,
        AW_WriteClean,
        AW_WriteBack,
        AW_Evict,
        AW_Barrier
    };

    sc_uint<4> getArType(axi4_l1_out_type src) {
        sc_uint<4> ret = AR_ReadNoSnoop;
        switch (src.ar_snoop) {
        case 0x0:
            if (src.ar_bar[0] == 1) {
                ret = AR_Barrier;
            } else if ((src.ar_domain[1] ^ src.ar_domain[0]) != 0) {
                ret = AR_ReadOnce;
            } else {
                ret = AR_ReadNoSnoop;
            }
            break;
        case 0x1:
            ret = AR_ReadShared;
            break;
        case 0x2:
            ret = AR_ReadClean;
            break;
        case 0x3:
            ret = AR_ReadNotSharedDirty;
            break;
        case 0x7:
            ret = AR_ReadUnique;
            break;
        case 0xB:
            ret = AR_CleanUnique;
            break;
        case 0xC:
            ret = AR_MakeUnique;
            break;
        case 0x8:
            ret = AR_CleanShared;
            break;
        case 0x9:
            ret = AR_CleanInvalid;
            break;
        case 0xD:
            ret = AR_MakeInvalid;
            break;
        case 0xE:
            ret = AR_DVM_Complete;
            break;
        case 0xF:
            ret = AR_DVM_Message;
            break;
        default:;
        }
        return ret;
    }

    sc_uint<3> getAwType(axi4_l1_out_type src) {
        sc_uint<3> ret = AW_WriteNoSnoop;
        switch (src.aw_snoop) {
        case 0x0:
            if (src.aw_bar[0] == 1) {
                ret = AW_Barrier;
            } else if ((src.aw_domain[1] ^ src.aw_domain[0]) != 0) {
                ret = AW_WriteUnique;
            } else {
                ret = AW_WriteNoSnoop;
            }
            break;
        case 0x1:
            ret = AW_WriteLineUnique;
            break;
        case 0x2:
            ret = AW_WriteClean;
            break;
        case 0x3:
            ret = AW_WriteBack;
            break;
        case 0x4:
            ret = AW_Evict;
            break;
        default:;
        }
        return ret;
    }

    static const int SRC_MUX_WIDTH = 5; // 4 cores + acp
    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<3>> srcid;

        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<sc_uint<5>> req_src;
        sc_signal<sc_uint<L2_REQ_TYPE_BITS>> req_type;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_wdata;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_wstrb;
    } r, v;

    void R_RESET(RegistersType &iv) {
        iv.state = Idle;
        iv.srcid = SRC_MUX_WIDTH;
        iv.req_addr = 0;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.req_src = 0;
        iv.req_type = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2DESTINATION_H__
