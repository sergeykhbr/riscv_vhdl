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
#include "../ambalib/apb_slv.h"
#include "api_core.h"

namespace debugger {

template<int width = 12>
SC_MODULE(apb_gpio) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_out<dev_config_type> o_cfg;                          // Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_in<sc_uint<width>> i_gpio;
    sc_out<sc_uint<width>> o_gpio_dir;                      // 1 as input; 0 as output
    sc_out<sc_uint<width>> o_gpio;
    sc_out<sc_uint<width>> o_irq;

    void comb();
    void registers();

    SC_HAS_PROCESS(apb_gpio);

    apb_gpio(sc_module_name name,
             bool async_reset);
    virtual ~apb_gpio();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct apb_gpio_registers {
        sc_signal<sc_uint<width>> input_val;
        sc_signal<sc_uint<width>> input_en;
        sc_signal<sc_uint<width>> output_en;
        sc_signal<sc_uint<width>> output_val;
        sc_signal<sc_uint<width>> ie;
        sc_signal<sc_uint<width>> ip;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    } v, r;

    void apb_gpio_r_reset(apb_gpio_registers &iv) {
        iv.input_val = 0;
        iv.input_en = ~0ul;
        iv.output_en = 0;
        iv.output_val = 0;
        iv.ie = 0;
        iv.ip = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;

    apb_slv *pslv0;

};

template<int width>
apb_gpio<width>::apb_gpio(sc_module_name name,
                          bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    i_gpio("i_gpio"),
    o_gpio_dir("o_gpio_dir"),
    o_gpio("o_gpio"),
    o_irq("o_irq") {

    async_reset_ = async_reset;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0", async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_GPIO);
    pslv0->i_clk(i_clk);
    pslv0->i_nrst(i_nrst);
    pslv0->i_mapinfo(i_mapinfo);
    pslv0->o_cfg(o_cfg);
    pslv0->i_apbi(i_apbi);
    pslv0->o_apbo(o_apbo);
    pslv0->o_req_valid(w_req_valid);
    pslv0->o_req_addr(wb_req_addr);
    pslv0->o_req_write(w_req_write);
    pslv0->o_req_wdata(wb_req_wdata);
    pslv0->i_resp_valid(r.resp_valid);
    pslv0->i_resp_rdata(r.resp_rdata);
    pslv0->i_resp_err(r.resp_err);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_apbi;
    sensitive << i_gpio;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.input_val;
    sensitive << r.input_en;
    sensitive << r.output_en;
    sensitive << r.output_val;
    sensitive << r.ie;
    sensitive << r.ip;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int width>
apb_gpio<width>::~apb_gpio() {
    if (pslv0) {
        delete pslv0;
    }
}

template<int width>
void apb_gpio<width>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, i_gpio, i_gpio.name());
        sc_trace(o_vcd, o_gpio_dir, o_gpio_dir.name());
        sc_trace(o_vcd, o_gpio, o_gpio.name());
        sc_trace(o_vcd, o_irq, o_irq.name());
        sc_trace(o_vcd, r.input_val, pn + ".r_input_val");
        sc_trace(o_vcd, r.input_en, pn + ".r_input_en");
        sc_trace(o_vcd, r.output_en, pn + ".r_output_en");
        sc_trace(o_vcd, r.output_val, pn + ".r_output_val");
        sc_trace(o_vcd, r.ie, pn + ".r_ie");
        sc_trace(o_vcd, r.ip, pn + ".r_ip");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

template<int width>
void apb_gpio<width>::comb() {
    sc_uint<32> vb_rdata;

    vb_rdata = 0;

    v = r;

    v.input_val = (i_gpio.read() & r.input_en.read());

    // Registers access:
    switch (wb_req_addr.read()(11, 2)) {
    case 0:                                                 // 0x00: RO input_val
        vb_rdata((width - 1), 0) = r.input_val;
        break;
    case 1:                                                 // 0x04: input_en
        vb_rdata((width - 1), 0) = r.input_en;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.input_en = wb_req_wdata.read()((width - 1), 0);
        }
        break;
    case 2:                                                 // 0x08: output_en
        vb_rdata((width - 1), 0) = r.output_en;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.output_en = wb_req_wdata.read()((width - 1), 0);
        }
        break;
    case 3:                                                 // 0x0C: output_val
        vb_rdata((width - 1), 0) = r.output_val;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.output_val = wb_req_wdata.read()((width - 1), 0);
        }
        break;
    case 4:                                                 // 0x10: ie
        vb_rdata((width - 1), 0) = r.ie;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.ie = wb_req_wdata.read()((width - 1), 0);
        }
        break;
    case 5:                                                 // 0x14: ip
        vb_rdata((width - 1), 0) = r.ip;
        if ((w_req_valid.read() == 1) && (w_req_write.read() == 1)) {
            v.ip = wb_req_wdata.read()((width - 1), 0);
        }
        break;
    default:
        break;
    }

    v.resp_valid = w_req_valid;
    v.resp_rdata = vb_rdata;
    v.resp_err = 0;

    if (!async_reset_ && i_nrst.read() == 0) {
        apb_gpio_r_reset(v);
    }

    o_gpio_dir = r.input_en;
    o_gpio = r.output_val;
    o_irq = (r.ie.read() & r.ip.read());
}

template<int width>
void apb_gpio<width>::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        apb_gpio_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

