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

#include "axi_slv.h"
#include "api_core.h"

namespace debugger {

axi_slv::axi_slv(sc_module_name name,
                 bool async_reset,
                 uint32_t vid,
                 uint32_t did)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    o_req_valid("o_req_valid"),
    o_req_addr("o_req_addr"),
    o_req_size("o_req_size"),
    o_req_write("o_req_write"),
    o_req_wdata("o_req_wdata"),
    o_req_wstrb("o_req_wstrb"),
    o_req_last("o_req_last"),
    i_req_ready("i_req_ready"),
    i_resp_valid("i_resp_valid"),
    i_resp_rdata("i_resp_rdata"),
    i_resp_err("i_resp_err") {

    async_reset_ = async_reset;
    vid_ = vid;
    did_ = did;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_xslvi;
    sensitive << i_req_ready;
    sensitive << i_resp_valid;
    sensitive << i_resp_rdata;
    sensitive << i_resp_err;
    sensitive << r.state;
    sensitive << r.req_valid;
    sensitive << r.req_addr;
    sensitive << r.req_write;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.req_xsize;
    sensitive << r.req_len;
    sensitive << r.req_user;
    sensitive << r.req_id;
    sensitive << r.req_burst;
    sensitive << r.req_last;
    sensitive << r.resp_valid;
    sensitive << r.resp_last;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void axi_slv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, o_req_valid, o_req_valid.name());
        sc_trace(o_vcd, o_req_addr, o_req_addr.name());
        sc_trace(o_vcd, o_req_size, o_req_size.name());
        sc_trace(o_vcd, o_req_write, o_req_write.name());
        sc_trace(o_vcd, o_req_wdata, o_req_wdata.name());
        sc_trace(o_vcd, o_req_wstrb, o_req_wstrb.name());
        sc_trace(o_vcd, o_req_last, o_req_last.name());
        sc_trace(o_vcd, i_req_ready, i_req_ready.name());
        sc_trace(o_vcd, i_resp_valid, i_resp_valid.name());
        sc_trace(o_vcd, i_resp_rdata, i_resp_rdata.name());
        sc_trace(o_vcd, i_resp_err, i_resp_err.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_valid, pn + ".r_req_valid");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_write, pn + ".r_req_write");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.req_xsize, pn + ".r_req_xsize");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.req_user, pn + ".r_req_user");
        sc_trace(o_vcd, r.req_id, pn + ".r_req_id");
        sc_trace(o_vcd, r.req_burst, pn + ".r_req_burst");
        sc_trace(o_vcd, r.req_last, pn + ".r_req_last");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_last, pn + ".r_resp_last");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

}

void axi_slv::comb() {
    sc_uint<12> vb_req_addr_next;
    bool v_req_last;
    dev_config_type vcfg;
    axi4_slave_out_type vxslvo;

    vb_req_addr_next = 0;
    v_req_last = 0;
    vcfg = dev_config_none;
    vxslvo = axi4_slave_out_none;

    v = r;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.read().addr_start;
    vcfg.addr_end = i_mapinfo.read().addr_end;
    vcfg.vid = vid_;
    vcfg.did = did_;

    vb_req_addr_next = (r.req_addr.read()(11, 0) + r.req_xsize.read());
    if (r.req_burst.read() == AXI_BURST_FIXED) {
        vb_req_addr_next = r.req_addr.read()(11, 0);
    } else if (r.req_burst.read() == AXI_BURST_WRAP) {
        // Wrap suppported only 2, 4, 8 or 16 Bytes. See ARMDeveloper spec.
        if (r.req_xsize.read() == 2) {
            vb_req_addr_next(11, 1) = r.req_addr.read()(11, 1);
        } else if (r.req_xsize.read() == 4) {
            vb_req_addr_next(11, 2) = r.req_addr.read()(11, 2);
        } else if (r.req_xsize.read() == 8) {
            vb_req_addr_next(11, 3) = r.req_addr.read()(11, 3);
        } else if (r.req_xsize.read() == 16) {
            vb_req_addr_next(11, 4) = r.req_addr.read()(11, 4);
        } else if (r.req_xsize.read() == 32) {
            // Optional (not in ARM spec)
            vb_req_addr_next(11, 5) = r.req_addr.read()(11, 5);
        }
    }

    v_req_last = (!r.req_len.read().or_reduce());
    v.req_last = v_req_last;

    switch (r.state.read()) {
    case State_Idle:
        v.req_valid = 0;
        v.req_write = 0;
        v.resp_valid = 0;
        v.resp_last = 0;
        v.resp_err = 0;
        vxslvo.aw_ready = 1;
        vxslvo.w_ready = 1;                                 // No burst AXILite ready
        vxslvo.ar_ready = (!i_xslvi.read().aw_valid);
        if (i_xslvi.read().aw_valid == 1) {
            v.req_addr = i_xslvi.read().aw_bits.addr;
            v.req_xsize = XSizeToBytes(i_xslvi.read().aw_bits.size);
            v.req_len = i_xslvi.read().aw_bits.len;
            v.req_burst = i_xslvi.read().aw_bits.burst;
            v.req_id = i_xslvi.read().aw_id;
            v.req_user = i_xslvi.read().aw_user;
            v.req_wdata = i_xslvi.read().w_data;            // AXI Lite compatible
            v.req_wstrb = i_xslvi.read().w_strb;
            if (i_xslvi.read().w_valid == 1) {
                // AXI Lite does not support burst transaction
                v.state = State_last_w;
                v.req_valid = 1;
                v.req_write = 1;
            } else {
                v.state = State_w;
            }
        } else if (i_xslvi.read().ar_valid == 1) {
            v.req_valid = 1;
            v.req_addr = i_xslvi.read().ar_bits.addr;
            v.req_xsize = XSizeToBytes(i_xslvi.read().ar_bits.size);
            v.req_len = i_xslvi.read().ar_bits.len;
            v.req_burst = i_xslvi.read().ar_bits.burst;
            v.req_id = i_xslvi.read().ar_id;
            v.req_user = i_xslvi.read().ar_user;
            v.state = State_addr_r;
        }
        break;
    case State_w:
        vxslvo.w_ready = 1;
        v.req_wdata = i_xslvi.read().w_data;
        v.req_wstrb = i_xslvi.read().w_strb;
        if (i_xslvi.read().w_valid == 1) {
            v.req_valid = 1;
            v.req_write = 1;
            if (r.req_len.read().or_reduce() == 1) {
                v.state = State_burst_w;
            } else {
                v.state = State_last_w;
            }
        }
        break;
    case State_burst_w:
        v.req_valid = i_xslvi.read().w_valid;
        vxslvo.w_ready = i_resp_valid;
        if ((i_xslvi.read().w_valid == 1) && (i_resp_valid.read() == 1)) {
            v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_req_addr_next);
            v.req_wdata = i_xslvi.read().w_data;
            v.req_wstrb = i_xslvi.read().w_strb;
            if (r.req_len.read().or_reduce() == 1) {
                v.req_len = (r.req_len.read() - 1);
            }
            if (r.req_len.read() == 1) {
                v.state = State_last_w;
            }
        }
        break;
    case State_last_w:
        // Wait cycle: w_ready is zero on the last write because it is laready accepted
        if (i_resp_valid.read() == 1) {
            v.req_valid = 0;
            v.req_write = 0;
            v.resp_err = i_resp_err;
            v.state = State_b;
        }
        break;
    case State_b:
        vxslvo.b_valid = 1;
        if (i_xslvi.read().b_ready == 1) {
            v.state = State_Idle;
        }
        break;
    case State_addr_r:
        // Setup address:
        if (i_req_ready.read() == 1) {
            if (r.req_len.read().or_reduce() == 1) {
                v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_req_addr_next);
                v.req_len = (r.req_len.read() - 1);
                v.state = State_addrdata_r;
            } else {
                v.req_valid = 0;
                v.state = State_data_r;
            }
        }
        break;
    case State_addrdata_r:
        v.resp_valid = i_resp_valid;
        v.resp_rdata = i_resp_rdata;
        v.resp_err = i_resp_err;
        if ((i_resp_valid.read() == 0) || (r.req_len.read().or_reduce() == 0)) {
            v.req_valid = 0;
            v.state = State_data_r;
        } else if (i_xslvi.read().r_ready == 0) {
            // Bus is not ready to accept read data
            v.req_valid = 0;
            v.state = State_out_r;
        } else if (i_req_ready.read() == 0) {
            // Slave device is not ready to accept burst request
            v.state = State_addr_r;
        } else {
            v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_req_addr_next);
            v.req_len = (r.req_len.read() - 1);
        }
        break;
    case State_data_r:
        if (i_resp_valid.read() == 1) {
            v.resp_valid = 1;
            v.resp_rdata = i_resp_rdata;
            v.resp_err = i_resp_err;
            v.resp_last = (!r.req_len.read().or_reduce());
            v.state = State_out_r;
        }
        break;
    case State_out_r:
        if (i_xslvi.read().r_ready == 1) {
            v.resp_valid = 0;
            v.resp_last = 0;
            if (r.req_len.read().or_reduce() == 1) {
                v.req_valid = 1;
                v.state = State_addr_r;
            } else {
                v.state = State_Idle;
            }
        }
        break;
    default:
        break;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        axi_slv_r_reset(v);
    }

    o_req_valid = r.req_valid;
    o_req_last = v_req_last;
    o_req_addr = r.req_addr;
    o_req_size = r.req_xsize;
    o_req_write = r.req_write;
    o_req_wdata = r.req_wdata;
    o_req_wstrb = r.req_wstrb;

    vxslvo.b_id = r.req_id;
    vxslvo.b_user = r.req_user;
    vxslvo.b_resp = (r.resp_err.read() << 1);
    vxslvo.r_valid = r.resp_valid;
    vxslvo.r_id = r.req_id;
    vxslvo.r_user = r.req_user;
    vxslvo.r_resp = (r.resp_err.read() << 1);
    vxslvo.r_data = r.resp_rdata;
    vxslvo.r_last = r.resp_last;
    o_xslvo = vxslvo;
    o_cfg = vcfg;
}

void axi_slv::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        axi_slv_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

