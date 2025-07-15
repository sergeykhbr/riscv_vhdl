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
    sensitive << r.rstate;
    sensitive << r.wstate;
    sensitive << r.ar_ready;
    sensitive << r.ar_addr;
    sensitive << r.ar_len;
    sensitive << r.ar_bytes;
    sensitive << r.ar_burst;
    sensitive << r.ar_id;
    sensitive << r.ar_user;
    sensitive << r.ar_last;
    sensitive << r.aw_ready;
    sensitive << r.aw_addr;
    sensitive << r.aw_bytes;
    sensitive << r.aw_burst;
    sensitive << r.aw_id;
    sensitive << r.aw_user;
    sensitive << r.w_last;
    sensitive << r.w_ready;
    sensitive << r.r_valid;
    sensitive << r.r_last;
    sensitive << r.r_data;
    sensitive << r.r_err;
    sensitive << r.r_data_buf;
    sensitive << r.r_err_buf;
    sensitive << r.r_last_buf;
    sensitive << r.b_err;
    sensitive << r.b_valid;
    sensitive << r.req_valid;
    sensitive << r.req_addr;
    sensitive << r.req_last;
    sensitive << r.req_write;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.req_bytes;
    sensitive << r.req_addr_buf;
    sensitive << r.req_last_buf;
    sensitive << r.req_wdata_buf;
    sensitive << r.req_wstrb_buf;
    sensitive << r.requested;
    sensitive << r.resp_last;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void axi_slv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
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
        sc_trace(o_vcd, r.rstate, pn + ".r.rstate");
        sc_trace(o_vcd, r.wstate, pn + ".r.wstate");
        sc_trace(o_vcd, r.ar_ready, pn + ".r.ar_ready");
        sc_trace(o_vcd, r.ar_addr, pn + ".r.ar_addr");
        sc_trace(o_vcd, r.ar_len, pn + ".r.ar_len");
        sc_trace(o_vcd, r.ar_bytes, pn + ".r.ar_bytes");
        sc_trace(o_vcd, r.ar_burst, pn + ".r.ar_burst");
        sc_trace(o_vcd, r.ar_id, pn + ".r.ar_id");
        sc_trace(o_vcd, r.ar_user, pn + ".r.ar_user");
        sc_trace(o_vcd, r.ar_last, pn + ".r.ar_last");
        sc_trace(o_vcd, r.aw_ready, pn + ".r.aw_ready");
        sc_trace(o_vcd, r.aw_addr, pn + ".r.aw_addr");
        sc_trace(o_vcd, r.aw_bytes, pn + ".r.aw_bytes");
        sc_trace(o_vcd, r.aw_burst, pn + ".r.aw_burst");
        sc_trace(o_vcd, r.aw_id, pn + ".r.aw_id");
        sc_trace(o_vcd, r.aw_user, pn + ".r.aw_user");
        sc_trace(o_vcd, r.w_last, pn + ".r.w_last");
        sc_trace(o_vcd, r.w_ready, pn + ".r.w_ready");
        sc_trace(o_vcd, r.r_valid, pn + ".r.r_valid");
        sc_trace(o_vcd, r.r_last, pn + ".r.r_last");
        sc_trace(o_vcd, r.r_data, pn + ".r.r_data");
        sc_trace(o_vcd, r.r_err, pn + ".r.r_err");
        sc_trace(o_vcd, r.r_data_buf, pn + ".r.r_data_buf");
        sc_trace(o_vcd, r.r_err_buf, pn + ".r.r_err_buf");
        sc_trace(o_vcd, r.r_last_buf, pn + ".r.r_last_buf");
        sc_trace(o_vcd, r.b_err, pn + ".r.b_err");
        sc_trace(o_vcd, r.b_valid, pn + ".r.b_valid");
        sc_trace(o_vcd, r.req_valid, pn + ".r.req_valid");
        sc_trace(o_vcd, r.req_addr, pn + ".r.req_addr");
        sc_trace(o_vcd, r.req_last, pn + ".r.req_last");
        sc_trace(o_vcd, r.req_write, pn + ".r.req_write");
        sc_trace(o_vcd, r.req_wdata, pn + ".r.req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r.req_wstrb");
        sc_trace(o_vcd, r.req_bytes, pn + ".r.req_bytes");
        sc_trace(o_vcd, r.req_addr_buf, pn + ".r.req_addr_buf");
        sc_trace(o_vcd, r.req_last_buf, pn + ".r.req_last_buf");
        sc_trace(o_vcd, r.req_wdata_buf, pn + ".r.req_wdata_buf");
        sc_trace(o_vcd, r.req_wstrb_buf, pn + ".r.req_wstrb_buf");
        sc_trace(o_vcd, r.requested, pn + ".r.requested");
        sc_trace(o_vcd, r.resp_last, pn + ".r.resp_last");
    }

}

void axi_slv::comb() {
    sc_uint<12> vb_ar_addr_next;
    sc_uint<12> vb_aw_addr_next;
    sc_uint<8> vb_ar_len_next;
    dev_config_type vcfg;
    axi4_slave_out_type vxslvo;

    v = r;
    vb_ar_addr_next = 0;
    vb_aw_addr_next = 0;
    vb_ar_len_next = 0;
    vcfg = dev_config_none;
    vxslvo = axi4_slave_out_none;

    vcfg.descrsize = PNP_CFG_DEV_DESCR_BYTES;
    vcfg.descrtype = PNP_CFG_TYPE_SLAVE;
    vcfg.addr_start = i_mapinfo.read().addr_start;
    vcfg.addr_end = i_mapinfo.read().addr_end;
    vcfg.vid = vid_;
    vcfg.did = did_;

    vb_ar_addr_next = (r.req_addr.read()(11, 0) + (0, r.ar_bytes.read()));
    if (r.ar_burst.read() == AXI_BURST_FIXED) {
        vb_ar_addr_next = r.req_addr.read()(11, 0);
    } else if (r.ar_burst.read() == AXI_BURST_WRAP) {
        // Wrap suppported only 2, 4, 8 or 16 Bytes. See ARMDeveloper spec.
        if (r.ar_bytes.read() == 2) {
            vb_ar_addr_next(11, 1) = r.req_addr.read()(11, 1);
        } else if (r.ar_bytes.read() == 4) {
            vb_ar_addr_next(11, 2) = r.req_addr.read()(11, 2);
        } else if (r.ar_bytes.read() == 8) {
            vb_ar_addr_next(11, 3) = r.req_addr.read()(11, 3);
        } else if (r.ar_bytes.read() == 16) {
            vb_ar_addr_next(11, 4) = r.req_addr.read()(11, 4);
        } else if (r.ar_bytes.read() == 32) {
            // Optional (not in ARM spec)
            vb_ar_addr_next(11, 5) = r.req_addr.read()(11, 5);
        }
    }
    vb_ar_len_next = (r.ar_len.read() - 1);

    vb_aw_addr_next = (r.req_addr.read()(11, 0) + (0, r.aw_bytes.read()));
    if (r.aw_burst.read() == AXI_BURST_FIXED) {
        vb_aw_addr_next = r.req_addr.read()(11, 0);
    } else if (r.aw_burst.read() == AXI_BURST_WRAP) {
        // Wrap suppported only 2, 4, 8 or 16 Bytes. See ARMDeveloper spec.
        if (r.aw_bytes.read() == 2) {
            vb_aw_addr_next(11, 1) = r.req_addr.read()(11, 1);
        } else if (r.aw_bytes.read() == 4) {
            vb_aw_addr_next(11, 2) = r.req_addr.read()(11, 2);
        } else if (r.aw_bytes.read() == 8) {
            vb_aw_addr_next(11, 3) = r.req_addr.read()(11, 3);
        } else if (r.aw_bytes.read() == 16) {
            vb_aw_addr_next(11, 4) = r.req_addr.read()(11, 4);
        } else if (r.aw_bytes.read() == 32) {
            // Optional (not in ARM spec)
            vb_aw_addr_next(11, 5) = r.req_addr.read()(11, 5);
        }
    }

    if ((i_xslvi.read().ar_valid & r.ar_ready.read()) == 1) {
        v.ar_ready = 0;
    }
    if ((i_xslvi.read().aw_valid & r.aw_ready.read()) == 1) {
        v.aw_ready = 0;
    }
    if ((i_xslvi.read().w_valid & r.w_ready.read()) == 1) {
        v.w_ready = 0;
    }
    if ((i_xslvi.read().r_ready & r.r_valid.read()) == 1) {
        v.r_err = 0;
        v.r_last = 0;
        v.r_valid = 0;
    }
    if ((i_xslvi.read().b_ready & r.b_valid.read()) == 1) {
        v.b_err = 0;
        v.b_valid = 0;
    }
    if ((r.req_valid.read() & i_req_ready.read()) == 1) {
        v.req_valid = 0;
        v.requested = 1;
    } else if (i_resp_valid.read() == 1) {
        v.requested = 0;
    }

    // Reading channel (write first):
    switch (r.rstate.read()) {
    case State_r_idle:
        v.ar_addr = (i_xslvi.read().ar_bits.addr - i_mapinfo.read().addr_start);
        v.ar_len = i_xslvi.read().ar_bits.len;
        v.ar_burst = i_xslvi.read().ar_bits.burst;
        v.ar_bytes = XSizeToBytes(i_xslvi.read().ar_bits.size);
        v.ar_last = (!i_xslvi.read().ar_bits.len.or_reduce());
        v.ar_id = i_xslvi.read().ar_id;
        v.ar_user = i_xslvi.read().ar_user;
        if ((r.ar_ready.read() == 1) && (i_xslvi.read().ar_valid == 1)) {
            if (((i_xslvi.read().aw_valid & r.aw_ready.read()) == 1) || (r.wstate.read().or_reduce() == 1)) {
                v.rstate = State_r_wait_writing;
            } else {
                v.rstate = State_r_addr;
                v.req_valid = 1;
                v.req_write = 0;
                v.req_addr = (i_xslvi.read().ar_bits.addr - i_mapinfo.read().addr_start);
                v.req_last = (!i_xslvi.read().ar_bits.len.or_reduce());
                v.req_bytes = XSizeToBytes(i_xslvi.read().ar_bits.size);
            }
        } else {
            v.ar_ready = 1;
        }
        break;
    case State_r_addr:
        if (i_req_ready.read() == 1) {
            v.resp_last = r.req_last.read();
            if (r.req_last.read() == 1) {
                v.rstate = State_r_resp_last;
            } else {
                v.rstate = State_r_pipe;
                v.ar_len = (r.ar_len.read() - 1);
                v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_ar_addr_next);
                v.req_last = (!vb_ar_len_next.or_reduce());
                v.req_valid = 1;
            }
        }
        break;
    case State_r_pipe:
        //   r_ready  | resp_valid | req_ready |
        //      0     |     0      |     0     | do nothing
        //      0     |     0      |     1     | --- cannot be second ack without resp --
        //      0     |     1      |     0     | r_wait_accept
        //      0     |     1      |     1     | r_wait_accept (-> bufferred)
        //      1     |     0      |     0     | do nothing
        //      1     |     0      |     1     | --- cannot be second ack without resp --
        //      1     |     1      |     0     | r_addr
        //      1     |     1      |     1     | stay here, latch new data
        if (i_resp_valid.read() == 1) {
            v.r_valid = 1;
            v.r_last = 0;
            v.r_data = i_resp_rdata.read();
            v.r_err = i_resp_err.read();
            if ((i_xslvi.read().r_ready == 1) && (i_req_ready.read() == 0)) {
                v.rstate = State_r_addr;
            } else if (i_xslvi.read().r_ready == 0) {
                v.rstate = State_r_wait_accept;
            }
        }
        if (i_req_ready.read() == 1) {
            v.resp_last = r.req_last.read();
            if (r.req_last.read() == 1) {
                v.rstate = State_r_resp_last;
            } else if (i_xslvi.read().r_ready == 0) {
                // Goto r_wait_accept without new request
            } else {
                v.ar_len = (r.ar_len.read() - 1);
                v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_ar_addr_next);
                v.req_last = (!vb_ar_len_next.or_reduce());
                v.req_valid = 1;
            }
        }
        break;
    case State_r_resp_last:
        if (i_resp_valid.read() == 1) {
            if ((r.r_valid.read() == 1) && (i_xslvi.read().r_ready == 0)) {
                v.r_data_buf = i_resp_rdata.read();
                v.r_err_buf = i_resp_err.read();
                v.r_last_buf = 1;
                v.rstate = State_r_buf;
            } else {
                v.r_valid = 1;
                v.r_data = i_resp_rdata.read();
                v.r_err = i_resp_err.read();
                v.r_last = 1;
                v.rstate = State_r_wait_accept;
            }
        }
        break;
    case State_r_wait_accept:
        if (i_xslvi.read().r_ready == 1) {
            if (r.r_last.read() == 1) {
                v.rstate = State_r_idle;
                v.r_last = 0;
            } else if (i_resp_valid.read() == 1) {
                v.r_valid = 1;
                v.r_data = i_resp_rdata.read();
                v.r_err = i_resp_err.read();
                v.r_last = r.resp_last.read();
            } else if ((r.req_valid.read() == 1) && (i_req_ready.read() == 0)) {
                // the last request still wasn't accepted since pipe stage
                v.rstate = State_r_addr;
            } else if ((r.req_valid.read() == 1) && (i_req_ready.read() == 1)) {
                if (r.req_last.read() == 1) {
                    v.rstate = State_r_resp_last;
                } else {
                    v.rstate = State_r_pipe;
                }
            } else if (r.requested.read() == 1) {
                // The latest one was already requested and request was accepeted
                // Just wait i_resp_valid here
            } else {
                v.rstate = State_r_addr;
                v.ar_len = (r.ar_len.read() - 1);
                v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_ar_addr_next);
                v.req_last = (!vb_ar_len_next.or_reduce());
                v.req_valid = 1;
            }
        } else if ((r.r_valid.read() == 1) && (r.r_last.read() == 0) && (i_resp_valid.read() == 1)) {
            // We already requested the last value but previous was not accepted yet
            v.r_data_buf = i_resp_rdata.read();
            v.r_err_buf = i_resp_err.read();
            v.r_last_buf = r.resp_last.read();
            v.rstate = State_r_buf;
        } else if ((r.r_last.read() == 0) && (i_resp_valid.read() == 1)) {
            // We recieve new data after some pause
            v.r_data = i_resp_rdata.read();
            v.r_err = i_resp_err.read();
            v.r_last = r.resp_last.read();
            v.r_valid = 1;
        }
        if (i_req_ready.read() == 1) {
            v.resp_last = r.req_last.read();
        }
        break;
    case State_r_buf:
        if (i_xslvi.read().r_ready == 1) {
            v.r_valid = 1;
            v.r_last = r.r_last_buf.read();
            v.r_data = r.r_data_buf.read();
            v.r_err = r.r_err_buf.read();
            v.rstate = State_r_wait_accept;
        }
        break;
    case State_r_wait_writing:
        if (((r.wstate.read().or_reduce() == 0) && (i_xslvi.read().aw_valid == 0))
                || ((r.req_valid.read() & r.req_last.read() & i_req_ready.read()) == 1)) {
            // End of writing, start reading
            v.req_valid = 1;
            v.req_write = 0;
            v.req_addr = r.ar_addr.read();
            v.req_bytes = r.ar_bytes.read();
            v.req_last = r.ar_last.read();
            v.rstate = State_r_addr;
        }
        break;
    default:
        v.rstate = State_r_idle;
        break;
    }

    // Writing channel:
    switch (r.wstate.read()) {
    case State_w_idle:
        v.aw_addr = (i_xslvi.read().aw_bits.addr - i_mapinfo.read().addr_start);
        v.aw_burst = i_xslvi.read().aw_bits.burst;
        v.aw_bytes = XSizeToBytes(i_xslvi.read().aw_bits.size);
        v.w_last = (!i_xslvi.read().aw_bits.len.or_reduce());
        v.aw_id = i_xslvi.read().aw_id;
        v.aw_user = i_xslvi.read().aw_user;
        if ((r.aw_ready.read() == 1) && (i_xslvi.read().aw_valid == 1)) {
            // Warning: Do not try to support AXI Light here!!
            //     It is Devil that overcomplicates your inteconnect and stuck your system
            //     when 2 masters (AXI and AXI Light) will try to write into the same slave.
            if (r.rstate.read().or_reduce() == 1) {
                v.wstate = State_w_wait_reading;
            } else {
                v.req_addr = (i_xslvi.read().aw_bits.addr - i_mapinfo.read().addr_start);
                v.req_bytes = XSizeToBytes(i_xslvi.read().aw_bits.size);
                v.wstate = State_w_req;
                v.w_ready = 1;
                v.req_write = 1;
            }
        } else {
            v.aw_ready = 1;
        }
        break;
    case State_w_req:
        if (i_xslvi.read().w_valid == 1) {
            v.w_ready = (i_req_ready.read() & (!i_xslvi.read().w_last));
            v.req_valid = 1;
            v.req_wdata = i_xslvi.read().w_data;
            v.req_wstrb = i_xslvi.read().w_strb;
            v.req_last = i_xslvi.read().w_last;
            v.wstate = State_w_pipe;
        }
        break;
    case State_w_pipe:
        v.w_ready = ((i_req_ready.read() | i_resp_valid.read()) & (!r.req_last.read()));
        if ((r.w_ready.read() == 1) && (i_xslvi.read().w_valid == 1)) {
            if (i_req_ready.read() == 0) {
                v.wstate = State_w_buf;
                v.w_ready = 0;
                v.req_addr_buf = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_aw_addr_next);
                v.req_wdata_buf = i_xslvi.read().w_data;
                v.req_wstrb_buf = i_xslvi.read().w_strb;
                v.req_last_buf = i_xslvi.read().w_last;
            } else {
                v.req_valid = 1;
                v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_aw_addr_next);
                v.req_wdata = i_xslvi.read().w_data;
                v.req_wstrb = i_xslvi.read().w_strb;
                v.req_last = i_xslvi.read().w_last;
            }
        }
        if ((r.req_valid.read() == 1) && (r.req_last.read() == 1) && (i_req_ready.read() == 1)) {
            v.wstate = State_w_resp;
        }
        if ((i_resp_valid.read() == 1) && (i_xslvi.read().w_valid == 0) && (r.req_valid.read() == 0)) {
            v.w_ready = 1;
            v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_aw_addr_next);
            v.wstate = State_w_req;
        }
        break;
    case State_w_buf:
        if (i_req_ready.read() == 1) {
            v.req_valid = 1;
            v.req_last = r.req_last_buf.read();
            v.req_addr = r.req_addr_buf.read();
            v.req_wdata = r.req_wdata_buf.read();
            v.req_wstrb = r.req_wstrb_buf.read();
            v.wstate = State_w_pipe;
        }
        break;
    case State_w_resp:
        if (i_resp_valid.read() == 1) {
            v.b_valid = 1;
            v.b_err = i_resp_err.read();
            v.w_last = 0;
            v.wstate = State_b;
        }
        break;
    case State_w_wait_reading:
        // ready to accept new data (no latched data)
        if ((r.rstate.read().or_reduce() == 0) || ((r.r_valid.read() & r.r_last.read() & i_xslvi.read().r_ready) == 1)) {
            v.w_ready = 1;
            v.req_write = 1;
            v.req_addr = r.aw_addr.read();
            v.req_bytes = r.aw_bytes.read();
            v.wstate = State_w_req;
        }
        break;
    case State_b:
        if ((r.b_valid.read() == 1) && (i_xslvi.read().b_ready == 1)) {
            v.b_valid = 0;
            v.b_err = 0;
            v.aw_ready = 1;
            v.wstate = State_w_idle;
        }
        break;
    default:
        v.wstate = State_w_idle;
        break;
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        axi_slv_r_reset(v);
    }

    o_req_valid = r.req_valid.read();
    o_req_last = r.req_last.read();
    o_req_addr = r.req_addr.read();
    o_req_size = r.req_bytes.read();
    o_req_write = r.req_write.read();
    o_req_wdata = r.req_wdata.read();
    o_req_wstrb = r.req_wstrb.read();

    vxslvo.ar_ready = r.ar_ready.read();
    vxslvo.r_valid = r.r_valid.read();
    vxslvo.r_id = r.ar_id.read();
    vxslvo.r_user = r.ar_user.read();
    vxslvo.r_resp = (r.r_err.read() << 1);
    vxslvo.r_data = r.r_data.read();
    vxslvo.r_last = r.r_last.read();
    vxslvo.aw_ready = r.aw_ready.read();
    vxslvo.w_ready = r.w_ready.read();
    vxslvo.b_valid = r.b_valid.read();
    vxslvo.b_id = r.aw_id.read();
    vxslvo.b_user = r.aw_user.read();
    vxslvo.b_resp = (r.b_err.read() << 1);
    o_xslvo = vxslvo;
    o_cfg = vcfg;
}

void axi_slv::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        axi_slv_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

