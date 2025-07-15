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
#include "../ambalib/types_amba.h"
#include "cdc_afifo.h"
#include "api_core.h"

namespace debugger {

template<int abits_depth = 2,                               // Depth of the address channels AR/AW/B
         int dbits_depth = 10>                              // Depth of the data channels R/W
SC_MODULE(afifo_xslv) {
 public:
    sc_in<bool> i_xslv_nrst;                                // Input requests reset (from master to this slave)
    sc_in<bool> i_xslv_clk;                                 // Input requests clock (from master to this slave)
    sc_in<axi4_slave_in_type> i_xslvi;                      // Input slave interface
    sc_out<axi4_slave_out_type> o_xslvo;                    // Response on input slave reuqest
    sc_in<bool> i_xmst_nrst;                                // Output request reset
    sc_in<bool> i_xmst_clk;                                 // Output request clock
    sc_out<axi4_slave_in_type> o_xmsto;                     // Output request to connected slave
    sc_in<axi4_slave_out_type> i_xmsti;                     // Response from the connected slave

    void comb();

    afifo_xslv(sc_module_name name);
    virtual ~afifo_xslv();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int AR_REQ_WIDTH = (CFG_SYSBUS_ADDR_BITS  // addr
            + 8  // len
            + 3  // size
            + 2  // burst
            + 1  // lock
            + 4  // cache
            + 3  // prot
            + 4  // qos
            + 4  // region
            + CFG_SYSBUS_ID_BITS  // ar_id
            + CFG_SYSBUS_USER_BITS  // ar_user
    );
    static const int AW_REQ_WIDTH = (CFG_SYSBUS_ADDR_BITS  // addr
            + 8  // len
            + 3  // size
            + 2  // burst
            + 1  // lock
            + 4  // cache
            + 3  // prot
            + 4  // qos
            + 4  // region
            + CFG_SYSBUS_ID_BITS  // aw_id
            + CFG_SYSBUS_USER_BITS  // aw_user
    );
    static const int W_REQ_WIDTH = (CFG_SYSBUS_DATA_BITS  // w_data
            + 1  // w_last
            + CFG_SYSBUS_DATA_BYTES  // w_strb
            + CFG_SYSBUS_USER_BITS  // w_user
    );
    static const int R_RESP_WIDTH = (2  // r_resp
            + CFG_SYSBUS_DATA_BITS  // r_data
            + 1  // r_last
            + CFG_SYSBUS_ID_BITS  // r_id
            + CFG_SYSBUS_USER_BITS  // r_user
    );
    static const int B_RESP_WIDTH = (2  // b_resp
            + CFG_SYSBUS_ID_BITS  // b_id
            + CFG_SYSBUS_USER_BITS  // b_user
    );

    sc_signal<bool> w_req_ar_valid_i;
    sc_signal<sc_biguint<AR_REQ_WIDTH>> wb_req_ar_payload_i;
    sc_signal<bool> w_req_ar_wready_o;
    sc_signal<bool> w_req_ar_rd_i;
    sc_signal<sc_biguint<AR_REQ_WIDTH>> wb_req_ar_payload_o;
    sc_signal<bool> w_req_ar_rvalid_o;
    sc_signal<bool> w_req_aw_valid_i;
    sc_signal<sc_biguint<AW_REQ_WIDTH>> wb_req_aw_payload_i;
    sc_signal<bool> w_req_aw_wready_o;
    sc_signal<bool> w_req_aw_rd_i;
    sc_signal<sc_biguint<AW_REQ_WIDTH>> wb_req_aw_payload_o;
    sc_signal<bool> w_req_aw_rvalid_o;
    sc_signal<bool> w_req_w_valid_i;
    sc_signal<sc_biguint<W_REQ_WIDTH>> wb_req_w_payload_i;
    sc_signal<bool> w_req_w_wready_o;
    sc_signal<bool> w_req_w_rd_i;
    sc_signal<sc_biguint<W_REQ_WIDTH>> wb_req_w_payload_o;
    sc_signal<bool> w_req_w_rvalid_o;
    sc_signal<bool> w_resp_r_valid_i;
    sc_signal<sc_biguint<R_RESP_WIDTH>> wb_resp_r_payload_i;
    sc_signal<bool> w_resp_r_wready_o;
    sc_signal<bool> w_resp_r_rd_i;
    sc_signal<sc_biguint<R_RESP_WIDTH>> wb_resp_r_payload_o;
    sc_signal<bool> w_resp_r_rvalid_o;
    sc_signal<bool> w_resp_b_valid_i;
    sc_signal<sc_biguint<B_RESP_WIDTH>> wb_resp_b_payload_i;
    sc_signal<bool> w_resp_b_wready_o;
    sc_signal<bool> w_resp_b_rd_i;
    sc_signal<sc_biguint<B_RESP_WIDTH>> wb_resp_b_payload_o;
    sc_signal<bool> w_resp_b_rvalid_o;

    cdc_afifo<abits_depth, AR_REQ_WIDTH> *req_ar;
    cdc_afifo<abits_depth, AW_REQ_WIDTH> *req_aw;
    cdc_afifo<dbits_depth, W_REQ_WIDTH> *req_w;
    cdc_afifo<dbits_depth, R_RESP_WIDTH> *resp_r;
    cdc_afifo<abits_depth, B_RESP_WIDTH> *resp_b;

};

template<int abits_depth, int dbits_depth>
afifo_xslv<abits_depth, dbits_depth>::afifo_xslv(sc_module_name name)
    : sc_module(name),
    i_xslv_nrst("i_xslv_nrst"),
    i_xslv_clk("i_xslv_clk"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    i_xmst_nrst("i_xmst_nrst"),
    i_xmst_clk("i_xmst_clk"),
    o_xmsto("o_xmsto"),
    i_xmsti("i_xmsti") {

    req_ar = 0;
    req_aw = 0;
    req_w = 0;
    resp_r = 0;
    resp_b = 0;

    req_ar = new cdc_afifo<abits_depth,
                           AR_REQ_WIDTH>("req_ar");
    req_ar->i_nrst(i_xslv_nrst);
    req_ar->i_wclk(i_xslv_clk);
    req_ar->i_wr(w_req_ar_valid_i);
    req_ar->i_wdata(wb_req_ar_payload_i);
    req_ar->o_wready(w_req_ar_wready_o);
    req_ar->i_rclk(i_xmst_clk);
    req_ar->i_rd(w_req_ar_rd_i);
    req_ar->o_rdata(wb_req_ar_payload_o);
    req_ar->o_rvalid(w_req_ar_rvalid_o);

    req_aw = new cdc_afifo<abits_depth,
                           AW_REQ_WIDTH>("req_aw");
    req_aw->i_nrst(i_xslv_nrst);
    req_aw->i_wclk(i_xslv_clk);
    req_aw->i_wr(w_req_aw_valid_i);
    req_aw->i_wdata(wb_req_aw_payload_i);
    req_aw->o_wready(w_req_aw_wready_o);
    req_aw->i_rclk(i_xmst_clk);
    req_aw->i_rd(w_req_aw_rd_i);
    req_aw->o_rdata(wb_req_aw_payload_o);
    req_aw->o_rvalid(w_req_aw_rvalid_o);

    req_w = new cdc_afifo<dbits_depth,
                          W_REQ_WIDTH>("req_w");
    req_w->i_nrst(i_xslv_nrst);
    req_w->i_wclk(i_xslv_clk);
    req_w->i_wr(w_req_w_valid_i);
    req_w->i_wdata(wb_req_w_payload_i);
    req_w->o_wready(w_req_w_wready_o);
    req_w->i_rclk(i_xmst_clk);
    req_w->i_rd(w_req_w_rd_i);
    req_w->o_rdata(wb_req_w_payload_o);
    req_w->o_rvalid(w_req_w_rvalid_o);

    resp_r = new cdc_afifo<dbits_depth,
                           R_RESP_WIDTH>("resp_r");
    resp_r->i_nrst(i_xmst_nrst);
    resp_r->i_wclk(i_xmst_clk);
    resp_r->i_wr(w_resp_r_valid_i);
    resp_r->i_wdata(wb_resp_r_payload_i);
    resp_r->o_wready(w_resp_r_wready_o);
    resp_r->i_rclk(i_xslv_clk);
    resp_r->i_rd(w_resp_r_rd_i);
    resp_r->o_rdata(wb_resp_r_payload_o);
    resp_r->o_rvalid(w_resp_r_rvalid_o);

    resp_b = new cdc_afifo<abits_depth,
                           B_RESP_WIDTH>("resp_b");
    resp_b->i_nrst(i_xmst_nrst);
    resp_b->i_wclk(i_xmst_clk);
    resp_b->i_wr(w_resp_b_valid_i);
    resp_b->i_wdata(wb_resp_b_payload_i);
    resp_b->o_wready(w_resp_b_wready_o);
    resp_b->i_rclk(i_xslv_clk);
    resp_b->i_rd(w_resp_b_rd_i);
    resp_b->o_rdata(wb_resp_b_payload_o);
    resp_b->o_rvalid(w_resp_b_rvalid_o);

    SC_METHOD(comb);
    sensitive << i_xslv_nrst;
    sensitive << i_xslv_clk;
    sensitive << i_xslvi;
    sensitive << i_xmst_nrst;
    sensitive << i_xmst_clk;
    sensitive << i_xmsti;
    sensitive << w_req_ar_valid_i;
    sensitive << wb_req_ar_payload_i;
    sensitive << w_req_ar_wready_o;
    sensitive << w_req_ar_rd_i;
    sensitive << wb_req_ar_payload_o;
    sensitive << w_req_ar_rvalid_o;
    sensitive << w_req_aw_valid_i;
    sensitive << wb_req_aw_payload_i;
    sensitive << w_req_aw_wready_o;
    sensitive << w_req_aw_rd_i;
    sensitive << wb_req_aw_payload_o;
    sensitive << w_req_aw_rvalid_o;
    sensitive << w_req_w_valid_i;
    sensitive << wb_req_w_payload_i;
    sensitive << w_req_w_wready_o;
    sensitive << w_req_w_rd_i;
    sensitive << wb_req_w_payload_o;
    sensitive << w_req_w_rvalid_o;
    sensitive << w_resp_r_valid_i;
    sensitive << wb_resp_r_payload_i;
    sensitive << w_resp_r_wready_o;
    sensitive << w_resp_r_rd_i;
    sensitive << wb_resp_r_payload_o;
    sensitive << w_resp_r_rvalid_o;
    sensitive << w_resp_b_valid_i;
    sensitive << wb_resp_b_payload_i;
    sensitive << w_resp_b_wready_o;
    sensitive << w_resp_b_rd_i;
    sensitive << wb_resp_b_payload_o;
    sensitive << w_resp_b_rvalid_o;
}

template<int abits_depth, int dbits_depth>
afifo_xslv<abits_depth, dbits_depth>::~afifo_xslv() {
    if (req_ar) {
        delete req_ar;
    }
    if (req_aw) {
        delete req_aw;
    }
    if (req_w) {
        delete req_w;
    }
    if (resp_r) {
        delete resp_r;
    }
    if (resp_b) {
        delete resp_b;
    }
}

template<int abits_depth, int dbits_depth>
void afifo_xslv<abits_depth, dbits_depth>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_xslv_nrst, i_xslv_nrst.name());
        sc_trace(o_vcd, i_xslv_clk, i_xslv_clk.name());
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, i_xmst_nrst, i_xmst_nrst.name());
        sc_trace(o_vcd, i_xmst_clk, i_xmst_clk.name());
        sc_trace(o_vcd, o_xmsto, o_xmsto.name());
        sc_trace(o_vcd, i_xmsti, i_xmsti.name());
    }

    if (req_ar) {
        req_ar->generateVCD(i_vcd, o_vcd);
    }
    if (req_aw) {
        req_aw->generateVCD(i_vcd, o_vcd);
    }
    if (req_w) {
        req_w->generateVCD(i_vcd, o_vcd);
    }
    if (resp_r) {
        resp_r->generateVCD(i_vcd, o_vcd);
    }
    if (resp_b) {
        resp_b->generateVCD(i_vcd, o_vcd);
    }
}

template<int abits_depth, int dbits_depth>
void afifo_xslv<abits_depth, dbits_depth>::comb() {
    axi4_slave_out_type vb_xslvo;
    axi4_slave_in_type vb_xmsto;

    // ar channel write side:
    w_req_ar_valid_i = i_xslvi.read().ar_valid;
    vb_xslvo.ar_ready = w_req_ar_wready_o.read();
    wb_req_ar_payload_i = (i_xslvi.read().ar_bits.addr,
            i_xslvi.read().ar_bits.len,
            i_xslvi.read().ar_bits.size,
            i_xslvi.read().ar_bits.burst,
            i_xslvi.read().ar_bits.lock,
            i_xslvi.read().ar_bits.cache,
            i_xslvi.read().ar_bits.prot,
            i_xslvi.read().ar_bits.qos,
            i_xslvi.read().ar_bits.region,
            i_xslvi.read().ar_id,
            i_xslvi.read().ar_user);
    // ar channel read side:
    vb_xmsto.ar_valid = w_req_ar_rvalid_o.read();
    w_req_ar_rd_i = i_xmsti.read().ar_ready;
    vb_xmsto.ar_bits.addr = wb_req_ar_payload_o.read()(84, 37);
    vb_xmsto.ar_bits.len = wb_req_ar_payload_o.read()(36, 29);
    vb_xmsto.ar_bits.size = wb_req_ar_payload_o.read()(28, 26);
    vb_xmsto.ar_bits.burst = wb_req_ar_payload_o.read()(25, 24);
    vb_xmsto.ar_bits.lock = wb_req_ar_payload_o.read()[23];
    vb_xmsto.ar_bits.cache = wb_req_ar_payload_o.read()(22, 19);
    vb_xmsto.ar_bits.prot = wb_req_ar_payload_o.read()(18, 16);
    vb_xmsto.ar_bits.qos = wb_req_ar_payload_o.read()(15, 12);
    vb_xmsto.ar_bits.region = wb_req_ar_payload_o.read()(11, 8);
    vb_xmsto.ar_id = wb_req_ar_payload_o.read()(7, 3);
    vb_xmsto.ar_user = wb_req_ar_payload_o.read()(2, 0);

    // aw channel write side:
    w_req_aw_valid_i = i_xslvi.read().aw_valid;
    vb_xslvo.aw_ready = w_req_aw_wready_o.read();
    wb_req_aw_payload_i = (i_xslvi.read().aw_bits.addr,
            i_xslvi.read().aw_bits.len,
            i_xslvi.read().aw_bits.size,
            i_xslvi.read().aw_bits.burst,
            i_xslvi.read().aw_bits.lock,
            i_xslvi.read().aw_bits.cache,
            i_xslvi.read().aw_bits.prot,
            i_xslvi.read().aw_bits.qos,
            i_xslvi.read().aw_bits.region,
            i_xslvi.read().aw_id,
            i_xslvi.read().aw_user);
    // aw channel read side:
    vb_xmsto.aw_valid = w_req_aw_rvalid_o.read();
    w_req_aw_rd_i = i_xmsti.read().aw_ready;
    vb_xmsto.aw_bits.addr = wb_req_aw_payload_o.read()(84, 37);
    vb_xmsto.aw_bits.len = wb_req_aw_payload_o.read()(36, 29);
    vb_xmsto.aw_bits.size = wb_req_aw_payload_o.read()(28, 26);
    vb_xmsto.aw_bits.burst = wb_req_aw_payload_o.read()(25, 24);
    vb_xmsto.aw_bits.lock = wb_req_aw_payload_o.read()[23];
    vb_xmsto.aw_bits.cache = wb_req_aw_payload_o.read()(22, 19);
    vb_xmsto.aw_bits.prot = wb_req_aw_payload_o.read()(18, 16);
    vb_xmsto.aw_bits.qos = wb_req_aw_payload_o.read()(15, 12);
    vb_xmsto.aw_bits.region = wb_req_aw_payload_o.read()(11, 8);
    vb_xmsto.aw_id = wb_req_aw_payload_o.read()(7, 3);
    vb_xmsto.aw_user = wb_req_aw_payload_o.read()(2, 0);

    // w channel write side:
    w_req_w_valid_i = i_xslvi.read().w_valid;
    vb_xslvo.w_ready = w_req_w_wready_o.read();
    wb_req_w_payload_i = (i_xslvi.read().w_data,
            i_xslvi.read().w_last,
            i_xslvi.read().w_strb,
            i_xslvi.read().w_user);
    // w channel read side:
    vb_xmsto.w_valid = w_req_w_rvalid_o.read();
    w_req_w_rd_i = i_xmsti.read().w_ready;
    vb_xmsto.w_data = wb_req_w_payload_o.read()(75, 12);
    vb_xmsto.w_last = wb_req_w_payload_o.read()[11];
    vb_xmsto.w_strb = wb_req_w_payload_o.read()(10, 3);
    vb_xmsto.w_user = wb_req_w_payload_o.read()(2, 0);

    // r channel write side:
    w_resp_r_valid_i = i_xmsti.read().r_valid;
    vb_xmsto.r_ready = w_resp_r_wready_o.read();
    wb_resp_r_payload_i = (i_xmsti.read().r_resp,
            i_xmsti.read().r_data,
            i_xmsti.read().r_last,
            i_xmsti.read().r_id,
            i_xmsti.read().r_user);
    // r channel read side:
    vb_xslvo.r_valid = w_resp_r_rvalid_o.read();
    w_resp_r_rd_i = i_xslvi.read().r_ready;
    vb_xslvo.r_resp = wb_resp_r_payload_o.read()(74, 73);
    vb_xslvo.r_data = wb_resp_r_payload_o.read()(72, 9);
    vb_xslvo.r_last = wb_resp_r_payload_o.read()[8];
    vb_xslvo.r_id = wb_resp_r_payload_o.read()(7, 3);
    vb_xslvo.r_user = wb_resp_r_payload_o.read()(2, 0);

    // b channel write side:
    w_resp_b_valid_i = i_xmsti.read().b_valid;
    vb_xmsto.b_ready = w_resp_b_wready_o.read();
    wb_resp_b_payload_i = (i_xmsti.read().b_resp,
            i_xmsti.read().b_id,
            i_xmsti.read().b_user);
    // b channel read side:
    vb_xslvo.b_valid = w_resp_b_rvalid_o.read();
    w_resp_b_rd_i = i_xslvi.read().b_ready;
    vb_xslvo.b_resp = wb_resp_b_payload_o.read()(9, 8);
    vb_xslvo.b_id = wb_resp_b_payload_o.read()(7, 3);
    vb_xslvo.b_user = wb_resp_b_payload_o.read()(2, 0);

    o_xslvo = vb_xslvo;
    o_xmsto = vb_xmsto;
}

}  // namespace debugger

