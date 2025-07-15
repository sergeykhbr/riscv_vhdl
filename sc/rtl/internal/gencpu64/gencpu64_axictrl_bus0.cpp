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

#include "gencpu64_axictrl_bus0.h"
#include "api_core.h"

namespace debugger {

gencpu64_axictrl_bus0::gencpu64_axictrl_bus0(sc_module_name name,
                                             bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    o_cfg("o_cfg"),
    i_xmsto("i_xmsto", CFG_BUS0_XMST_TOTAL),
    o_xmsti("o_xmsti", CFG_BUS0_XMST_TOTAL),
    i_xslvo("i_xslvo", CFG_BUS0_XSLV_TOTAL),
    o_xslvi("o_xslvi", CFG_BUS0_XSLV_TOTAL),
    o_mapinfo("o_mapinfo", CFG_BUS0_XSLV_TOTAL) {

    async_reset_ = async_reset;
    xdef0 = 0;

    xdef0 = new axi_slv("xdef0",
                         async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_AXI_INTERCONNECT);
    xdef0->i_clk(i_clk);
    xdef0->i_nrst(i_nrst);
    xdef0->i_mapinfo(wb_def_mapinfo);
    xdef0->o_cfg(o_cfg);
    xdef0->i_xslvi(wb_def_xslvi);
    xdef0->o_xslvo(wb_def_xslvo);
    xdef0->o_req_valid(w_def_req_valid);
    xdef0->o_req_addr(wb_def_req_addr);
    xdef0->o_req_size(wb_def_req_size);
    xdef0->o_req_write(w_def_req_write);
    xdef0->o_req_wdata(wb_def_req_wdata);
    xdef0->o_req_wstrb(wb_def_req_wstrb);
    xdef0->o_req_last(w_def_req_last);
    xdef0->i_req_ready(w_def_req_ready);
    xdef0->i_resp_valid(w_def_resp_valid);
    xdef0->i_resp_rdata(wb_def_resp_rdata);
    xdef0->i_resp_err(w_def_resp_err);

    SC_METHOD(comb);
    sensitive << i_nrst;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        sensitive << i_xmsto[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        sensitive << i_xslvo[i];
    }
    sensitive << wb_def_xslvi;
    sensitive << wb_def_xslvo;
    sensitive << wb_def_mapinfo;
    sensitive << w_def_req_valid;
    sensitive << wb_def_req_addr;
    sensitive << wb_def_req_size;
    sensitive << w_def_req_write;
    sensitive << wb_def_req_wdata;
    sensitive << wb_def_req_wstrb;
    sensitive << w_def_req_last;
    sensitive << w_def_req_ready;
    sensitive << w_def_resp_valid;
    sensitive << wb_def_resp_rdata;
    sensitive << w_def_resp_err;
    sensitive << r.w_select;
    sensitive << r.w_active;
    sensitive << r.r_def_valid;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

gencpu64_axictrl_bus0::~gencpu64_axictrl_bus0() {
    if (xdef0) {
        delete xdef0;
    }
}

void gencpu64_axictrl_bus0::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, r.w_select, pn + ".r.w_select");
        sc_trace(o_vcd, r.w_active, pn + ".r.w_active");
        sc_trace(o_vcd, r.r_def_valid, pn + ".r.r_def_valid");
    }

    if (xdef0) {
        xdef0->generateVCD(i_vcd, o_vcd);
    }
}

void gencpu64_axictrl_bus0::comb() {
    axi4_master_in_type vmsti[CFG_BUS0_XMST_TOTAL];
    axi4_master_out_type vmsto[CFG_BUS0_XMST_TOTAL];
    axi4_slave_in_type vslvi[CFG_BUS0_XSLV_TOTAL];
    axi4_slave_out_type vslvo[CFG_BUS0_XSLV_TOTAL];
    sc_uint<(CFG_BUS0_XMST_TOTAL * CFG_BUS0_XSLV_TOTAL)> vb_ar_select;
    sc_uint<((CFG_BUS0_XMST_TOTAL + 1) * CFG_BUS0_XSLV_TOTAL)> vb_ar_available;
    sc_uint<CFG_BUS0_XMST_TOTAL> vb_ar_hit;
    sc_uint<(CFG_BUS0_XMST_TOTAL * CFG_BUS0_XSLV_TOTAL)> vb_aw_select;
    sc_uint<((CFG_BUS0_XMST_TOTAL + 1) * CFG_BUS0_XSLV_TOTAL)> vb_aw_available;
    sc_uint<CFG_BUS0_XMST_TOTAL> vb_aw_hit;
    sc_uint<(CFG_BUS0_XMST_TOTAL * CFG_BUS0_XSLV_LOG2_TOTAL)> vb_w_select;
    sc_uint<CFG_BUS0_XMST_TOTAL> vb_w_active;

    v = r;
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        vmsti[i] = axi4_master_in_none;
    }
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        vmsto[i] = axi4_master_out_none;
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        vslvi[i] = axi4_slave_in_none;
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        vslvo[i] = axi4_slave_out_none;
    }
    vb_ar_select = 0;
    vb_ar_available = ~0ull;
    vb_ar_hit = 0;
    vb_aw_select = 0;
    vb_aw_available = ~0ull;
    vb_aw_hit = 0;
    vb_w_select = 0;
    vb_w_active = 0;


    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        vmsto[i] = i_xmsto[i].read();                       // Cannot read vector item from port in systemc
        vmsti[i] = axi4_master_in_none;
    }

    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        vslvo[i] = i_xslvo[i].read();                       // Cannot read vector item from port in systemc
        vslvi[i] = axi4_slave_in_none;
    }
    // Local unmapped slots:
    vslvo[(CFG_BUS0_XSLV_TOTAL - 1)] = wb_def_xslvo.read();

    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        for (int ii = 0; ii < (CFG_BUS0_XSLV_TOTAL - 1); ii++) {
            // Connect AR channel
            if (((CFG_BUS0_MAP[ii].addr_start >> 12) <= vmsto[i].ar_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12))
                    && (vmsto[i].ar_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12) < (CFG_BUS0_MAP[ii].addr_end >> 12))) {
                vb_ar_hit[i] = vmsto[i].ar_valid;
                vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] = (vmsto[i].ar_valid && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
                // Update availability bit for the next master and this slave:
                vb_ar_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = ((!vmsto[i].ar_valid) && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
            } else {
                vb_ar_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)];
            }

            // Connect AW channel
            if (((CFG_BUS0_MAP[ii].addr_start >> 12) <= vmsto[i].aw_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12))
                    && (vmsto[i].aw_bits.addr((CFG_SYSBUS_ADDR_BITS - 1), 12) < (CFG_BUS0_MAP[ii].addr_end >> 12))) {
                vb_aw_hit[i] = vmsto[i].aw_valid;
                vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] = (vmsto[i].aw_valid && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
                // Update availability bit for the next master and this slave:
                vb_aw_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = ((!vmsto[i].aw_valid) && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)]);
            } else {
                vb_aw_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + ii)] = vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + ii)];
            }
        }
    }

    // access to unmapped slave:
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        if ((vmsto[i].ar_valid == 1) && (vb_ar_hit[i] == 0)) {
            vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = (vmsto[i].ar_valid && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);
        }
        vb_ar_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = ((!vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))])
                && vb_ar_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);

        if ((vmsto[i].aw_valid == 1) && (vb_aw_hit[i] == 0)) {
            vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = (vmsto[i].aw_valid && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);
        }
        vb_aw_available[(((i + 1) * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))] = ((!vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))])
                && vb_aw_available[((i * CFG_BUS0_XSLV_TOTAL) + (CFG_BUS0_XSLV_TOTAL - 1))]);
    }

    vb_w_select = r.w_select.read();
    vb_w_active = r.w_active.read();
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        for (int ii = 0; ii < CFG_BUS0_XSLV_TOTAL; ii++) {
            if (vb_ar_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] == 1) {
                vmsti[i].ar_ready = vslvo[ii].ar_ready;
                vslvi[ii].ar_valid = vmsto[i].ar_valid;
                vslvi[ii].ar_bits = vmsto[i].ar_bits;
                vslvi[ii].ar_user = vmsto[i].ar_user;
                vslvi[ii].ar_id = (vmsto[i].ar_id, static_cast<sc_uint<CFG_BUS0_XMST_LOG2_TOTAL>>(i));
            }
            if (vb_aw_select[((i * CFG_BUS0_XSLV_TOTAL) + ii)] == 1) {
                vmsti[i].aw_ready = vslvo[ii].aw_ready;
                vslvi[ii].aw_valid = vmsto[i].aw_valid;
                vslvi[ii].aw_bits = vmsto[i].aw_bits;
                vslvi[ii].aw_user = vmsto[i].aw_user;
                vslvi[ii].aw_id = (vmsto[i].aw_id, static_cast<sc_uint<CFG_BUS0_XMST_LOG2_TOTAL>>(i));
                if (vslvo[ii].aw_ready == 1) {
                    // Switch W-channel index to future w-transaction without id
                    vb_w_select((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)) = static_cast<sc_uint<CFG_BUS0_XSLV_LOG2_TOTAL>>(ii);
                    vb_w_active[i] = 1;
                }
            }
        }
    }
    v.w_select = vb_w_select;

    // W-channel
    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        if ((vmsto[i].w_valid == 1) && (r.w_active.read()[i] == 1)) {
            vmsti[i].w_ready = vslvo[r.w_select.read()((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)).to_int()].w_ready;
            vslvi[r.w_select.read()((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)).to_int()].w_valid = vmsto[i].w_valid;
            vslvi[r.w_select.read()((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)).to_int()].w_data = vmsto[i].w_data;
            vslvi[r.w_select.read()((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)).to_int()].w_strb = vmsto[i].w_strb;
            vslvi[r.w_select.read()((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)).to_int()].w_last = vmsto[i].w_last;
            vslvi[r.w_select.read()((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)).to_int()].w_user = vmsto[i].w_user;
            if ((vmsto[i].w_last == 1)
                    && (vslvo[r.w_select.read()((i * CFG_BUS0_XSLV_LOG2_TOTAL) + CFG_BUS0_XSLV_LOG2_TOTAL - 1, (i * CFG_BUS0_XSLV_LOG2_TOTAL)).to_int()].w_ready == 1)) {
                vb_w_active[i] = 0;
            }
        }
    }
    v.w_active = vb_w_active;

    // B-channel
    for (int ii = 0; ii < CFG_BUS0_XSLV_TOTAL; ii++) {
        if (vslvo[ii].b_valid == 1) {
            vslvi[ii].b_ready = vmsto[vslvo[ii].b_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].b_ready;
            vmsti[vslvo[ii].b_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].b_valid = vslvo[ii].b_valid;
            vmsti[vslvo[ii].b_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].b_resp = vslvo[ii].b_resp;
            vmsti[vslvo[ii].b_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].b_user = vslvo[ii].b_user;
            vmsti[vslvo[ii].b_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].b_id = vslvo[ii].b_id((CFG_SYSBUS_ID_BITS - 1), CFG_BUS0_XMST_LOG2_TOTAL);
        }
    }

    // R-channel
    for (int ii = 0; ii < CFG_BUS0_XSLV_TOTAL; ii++) {
        if (vslvo[ii].r_valid == 1) {
            vslvi[ii].r_ready = vmsto[vslvo[ii].r_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].r_ready;
            vmsti[vslvo[ii].r_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].r_valid = vslvo[ii].r_valid;
            vmsti[vslvo[ii].r_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].r_data = vslvo[ii].r_data;
            vmsti[vslvo[ii].r_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].r_last = vslvo[ii].r_last;
            vmsti[vslvo[ii].r_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].r_resp = vslvo[ii].r_resp;
            vmsti[vslvo[ii].r_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].r_user = vslvo[ii].r_user;
            vmsti[vslvo[ii].r_id((CFG_BUS0_XMST_LOG2_TOTAL - 1), 0).to_int()].r_id = vslvo[ii].r_id((CFG_SYSBUS_ID_BITS - 1), CFG_BUS0_XMST_LOG2_TOTAL);
        }
    }

    w_def_req_ready = 1;
    v.r_def_valid = w_def_req_valid.read();
    w_def_resp_valid = r.r_def_valid.read();
    wb_def_resp_rdata = ~0ull;
    w_def_resp_err = 1;

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        gencpu64_axictrl_bus0_r_reset(v);
    }

    for (int i = 0; i < CFG_BUS0_XMST_TOTAL; i++) {
        o_xmsti[i] = vmsti[i];
    }
    for (int i = 0; i < CFG_BUS0_XSLV_TOTAL; i++) {
        o_xslvi[i] = vslvi[i];
        o_mapinfo[i] = CFG_BUS0_MAP[i];
    }
    wb_def_xslvi = vslvi[(CFG_BUS0_XSLV_TOTAL - 1)];
    wb_def_mapinfo = CFG_BUS0_MAP[(CFG_BUS0_XSLV_TOTAL - 1)];
}

void gencpu64_axictrl_bus0::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        gencpu64_axictrl_bus0_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

