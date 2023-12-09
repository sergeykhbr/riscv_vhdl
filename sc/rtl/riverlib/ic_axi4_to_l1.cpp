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

#include "ic_axi4_to_l1.h"
#include "api_core.h"

namespace debugger {

ic_axi4_to_l1::ic_axi4_to_l1(sc_module_name name,
                             bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_xmsto("i_xmsto"),
    o_xmsti("o_xmsti"),
    i_l1i("i_l1i"),
    o_l1o("o_l1o") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xmsto;
    sensitive << i_l1i;
    sensitive << r.state;
    sensitive << r.req_addr;
    sensitive << r.req_id;
    sensitive << r.req_user;
    sensitive << r.req_wstrb;
    sensitive << r.req_wdata;
    sensitive << r.req_len;
    sensitive << r.req_size;
    sensitive << r.req_prot;
    sensitive << r.writing;
    sensitive << r.read_modify_write;
    sensitive << r.line_data;
    sensitive << r.line_wstrb;
    sensitive << r.resp_data;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void ic_axi4_to_l1::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_xmsto, i_xmsto.name());
        sc_trace(o_vcd, o_xmsti, o_xmsti.name());
        sc_trace(o_vcd, i_l1i, i_l1i.name());
        sc_trace(o_vcd, o_l1o, o_l1o.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_id, pn + ".r_req_id");
        sc_trace(o_vcd, r.req_user, pn + ".r_req_user");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.req_size, pn + ".r_req_size");
        sc_trace(o_vcd, r.req_prot, pn + ".r_req_prot");
        sc_trace(o_vcd, r.writing, pn + ".r_writing");
        sc_trace(o_vcd, r.read_modify_write, pn + ".r_read_modify_write");
        sc_trace(o_vcd, r.line_data, pn + ".r_line_data");
        sc_trace(o_vcd, r.line_wstrb, pn + ".r_line_wstrb");
        sc_trace(o_vcd, r.resp_data, pn + ".r_resp_data");
    }

}

void ic_axi4_to_l1::comb() {
    axi4_master_in_type vb_xmsti;
    axi4_l1_out_type vb_l1o;
    sc_uint<(CFG_LOG2_L1CACHE_BYTES_PER_LINE - 3)> idx;     // request always 64 bits
    sc_uint<XSIZE_TOTAL> vb_req_xbytes;
    sc_uint<64> vb_req_mask;
    sc_biguint<L1CACHE_LINE_BITS> vb_r_data_modified;
    sc_uint<L1CACHE_BYTES_PER_LINE> vb_line_wstrb;
    sc_uint<64> vb_resp_data;
    sc_uint<CFG_SYSBUS_ADDR_BITS> t_req_addr;

    vb_xmsti = axi4_master_in_none;
    vb_l1o = axi4_l1_out_none;
    idx = 0;
    vb_req_xbytes = 0;
    vb_req_mask = 0;
    vb_r_data_modified = 0;
    vb_line_wstrb = 0;
    vb_resp_data = 0;
    t_req_addr = 0;

    v = r;

    vb_xmsti = axi4_master_in_none;
    vb_l1o = axi4_l1_out_none;
    t_req_addr = r.req_addr;

    idx = r.req_addr.read()((CFG_LOG2_L1CACHE_BYTES_PER_LINE - 1), 3);
    vb_req_xbytes = XSizeToBytes(r.req_size);

    vb_req_mask = 0;
    for (int i = 0; i < 8; i++) {
        if (r.req_wstrb.read()[i] == 1) {
            vb_req_mask((8 * i) + 8- 1, (8 * i)) = 0xFF;
        }
    }

    vb_resp_data = i_l1i.read().r_data((idx * 64) + 64 - 1, (idx * 64));

    vb_r_data_modified = i_l1i.read().r_data;
    vb_r_data_modified((idx * 64) + 64- 1, (idx * 64)) = ((i_l1i.read().r_data((idx * 64) + 64 - 1, (idx * 64)) & (~vb_req_mask))
            | (r.req_wdata.read() & vb_req_mask));

    vb_line_wstrb = 0;
    vb_line_wstrb((idx * 8) + 8- 1, (idx * 8)) = r.req_wstrb;

    switch (r.state.read()) {
    case Idle:
        vb_xmsti.ar_ready = 1;
        vb_xmsti.aw_ready = 1;
        v.read_modify_write = 0;
        v.writing = 0;
        if (i_xmsto.read().aw_valid == 1) {
            // Convert AXI4 to AXI4Lite used in L1
            v.req_id = i_xmsto.read().aw_id;
            v.req_user = i_xmsto.read().aw_user;
            v.req_addr = i_xmsto.read().aw_bits.addr;
            v.req_size = i_xmsto.read().aw_bits.size;
            v.req_len = i_xmsto.read().aw_bits.len;
            v.req_prot = i_xmsto.read().aw_bits.prot;
            v.writing = 1;
            v.state = WriteDataAccept;
        } else if (i_xmsto.read().ar_valid == 1) {
            v.req_id = i_xmsto.read().ar_id;
            v.req_user = i_xmsto.read().ar_user;
            v.req_addr = i_xmsto.read().ar_bits.addr;
            v.req_size = i_xmsto.read().ar_bits.size;
            v.req_len = i_xmsto.read().ar_bits.len;
            v.req_prot = i_xmsto.read().ar_bits.prot;
            v.state = ReadLineRequest;
        }
        break;
    case WriteDataAccept:
        vb_xmsti.w_ready = 1;
        v.req_wdata = i_xmsto.read().w_data;
        v.req_wstrb = i_xmsto.read().w_strb;
        if (i_xmsto.read().w_valid == 1) {
            // Cachable memory read and make unique to modify line
            v.read_modify_write = 1;
            v.state = ReadLineRequest;
        }
        break;
    case ReadLineRequest:
        vb_l1o.ar_valid = 1;
        vb_l1o.ar_bits.addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), CFG_LOG2_L1CACHE_BYTES_PER_LINE) << CFG_LOG2_L1CACHE_BYTES_PER_LINE);
        vb_l1o.ar_bits.cache = ARCACHE_WRBACK_READ_ALLOCATE;
        vb_l1o.ar_bits.size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
        vb_l1o.ar_bits.len = 0;
        vb_l1o.ar_bits.prot = r.req_prot;
        vb_l1o.ar_snoop = ARSNOOP_READ_MAKE_UNIQUE;
        vb_l1o.ar_id = r.req_id;
        vb_l1o.ar_user = r.req_user;
        if (i_l1i.read().ar_ready == 1) {
            v.state = WaitReadLineResponse;
        }
        break;
    case WaitReadLineResponse:
        vb_l1o.r_ready = 1;
        v.line_data = i_l1i.read().r_data;
        v.resp_data = vb_resp_data;
        if (i_l1i.read().r_valid == 1) {
            if (r.read_modify_write.read() == 1) {
                v.line_data = vb_r_data_modified;
                v.line_wstrb = ~0ull;
                v.state = WriteLineRequest;
            } else {
                v.state = WaitReadAccept;
            }
        }
        break;
    case WriteLineRequest:
        vb_l1o.aw_valid = 1;
        vb_l1o.aw_bits.addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), CFG_LOG2_L1CACHE_BYTES_PER_LINE) << CFG_LOG2_L1CACHE_BYTES_PER_LINE);
        vb_l1o.aw_bits.cache = AWCACHE_DEVICE_NON_BUFFERABLE;
        vb_l1o.aw_bits.size = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
        vb_l1o.aw_bits.len = 0;
        vb_l1o.aw_bits.prot = r.req_prot;
        vb_l1o.aw_snoop = AWSNOOP_WRITE_NO_SNOOP;           // offloading non-cached always
        vb_l1o.aw_id = r.req_id;
        vb_l1o.aw_user = r.req_user;
        // axi lite for L2-cache
        vb_l1o.w_valid = 1;
        vb_l1o.w_last = 1;
        vb_l1o.w_data = r.line_data;
        vb_l1o.w_strb = r.line_wstrb;
        if ((i_l1i.read().aw_ready == 1) && (i_l1i.read().w_ready == 1)) {
            if (r.req_len.read().or_reduce() == 0) {
                v.state = WaitWriteConfirmResponse;
            } else {
                v.state = CheckBurst;
            }
        }
        break;
    case WaitWriteConfirmResponse:
        vb_l1o.b_ready = 1;
        if (i_l1i.read().b_valid == 1) {
            v.state = WaitWriteAccept;
        }
        break;
    case WaitWriteAccept:
        vb_xmsti.b_valid = 1;
        vb_xmsti.b_id = r.req_id;
        vb_xmsti.b_user = r.req_user;
        if (i_xmsto.read().b_ready == 1) {
            v.state = Idle;
        }
        break;
    case WaitReadAccept:
        vb_xmsti.r_valid = 1;
        vb_xmsti.r_data = r.resp_data;
        vb_xmsti.r_last = (~r.req_len.read().or_reduce());
        vb_xmsti.r_id = r.req_id;
        vb_xmsti.r_user = r.req_user;
        if (i_xmsto.read().r_ready == 1) {
            v.state = CheckBurst;
        }
        break;
    case CheckBurst:
        if (r.req_len.read().or_reduce() == 0) {
            v.state = Idle;
        } else {
            // Burst transaction to support external DMA engine
            v.req_len = (r.req_len.read() - 1);
            t_req_addr(11, 0) = (r.req_addr.read()(11, 0) + vb_req_xbytes);
            v.req_addr = t_req_addr;
            v.read_modify_write = 0;
            if (r.writing.read() == 1) {
                v.state = WriteDataAccept;
            } else {
                v.state = ReadLineRequest;
            }
        }
        break;
    default:
        break;
    }

    o_xmsti = vb_xmsti;
    o_l1o = vb_l1o;
}

void ic_axi4_to_l1::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        ic_axi4_to_l1_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

