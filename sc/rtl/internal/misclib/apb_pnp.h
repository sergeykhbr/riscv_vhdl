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
#include "../ambalib/types_pnp.h"
#include "../ambalib/apb_slv.h"
#include "api_core.h"

namespace debugger {

template<int cfg_slots = 1>
SC_MODULE(apb_pnp) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<mapinfo_type> i_mapinfo;                          // interconnect slot information
    sc_vector<sc_in<dev_config_type>> i_cfg;                // Device descriptors vector
    sc_out<dev_config_type> o_cfg;                          // PNP Device descriptor
    sc_in<apb_in_type> i_apbi;                              // APB  Slave to Bridge interface
    sc_out<apb_out_type> o_apbo;                            // APB Bridge to Slave interface
    sc_out<bool> o_irq;

    void comb();
    void registers();

    apb_pnp(sc_module_name name,
            bool async_reset,
            sc_uint<32> hwid,
            int cpu_max,
            int l2cache_ena,
            int plic_irq_max);
    virtual ~apb_pnp();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    sc_uint<32> hwid_;
    int cpu_max_;
    int l2cache_ena_;
    int plic_irq_max_;

    struct apb_pnp_registers {
        sc_signal<sc_uint<32>> fw_id;
        sc_signal<sc_uint<32>> idt_l;
        sc_signal<sc_uint<32>> idt_m;
        sc_signal<sc_uint<32>> malloc_addr_l;
        sc_signal<sc_uint<32>> malloc_addr_m;
        sc_signal<sc_uint<32>> malloc_size_l;
        sc_signal<sc_uint<32>> malloc_size_m;
        sc_signal<sc_uint<32>> fwdbg1;
        sc_signal<sc_uint<32>> fwdbg2;
        sc_signal<sc_uint<32>> fwdbg3;
        sc_signal<sc_uint<32>> fwdbg4;
        sc_signal<sc_uint<32>> fwdbg5;
        sc_signal<sc_uint<32>> fwdbg6;
        sc_signal<bool> irq;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<32>> resp_rdata;
        sc_signal<bool> resp_err;
    };

    void apb_pnp_r_reset(apb_pnp_registers& iv) {
        iv.fw_id = 0;
        iv.idt_l = 0;
        iv.idt_m = 0;
        iv.malloc_addr_l = 0;
        iv.malloc_addr_m = 0;
        iv.malloc_size_l = 0;
        iv.malloc_size_m = 0;
        iv.fwdbg1 = 0;
        iv.fwdbg2 = 0;
        iv.fwdbg3 = 0;
        iv.fwdbg4 = 0;
        iv.fwdbg5 = 0;
        iv.fwdbg6 = 0;
        iv.irq = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0;
        iv.resp_err = 0;
    }

    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<32>> wb_req_addr;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<32>> wb_req_wdata;
    apb_pnp_registers v;
    apb_pnp_registers r;

    apb_slv *pslv0;

};

template<int cfg_slots>
apb_pnp<cfg_slots>::apb_pnp(sc_module_name name,
                            bool async_reset,
                            sc_uint<32> hwid,
                            int cpu_max,
                            int l2cache_ena,
                            int plic_irq_max)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    i_cfg("i_cfg", SOC_PNP_TOTAL),
    o_cfg("o_cfg"),
    i_apbi("i_apbi"),
    o_apbo("o_apbo"),
    o_irq("o_irq") {

    async_reset_ = async_reset;
    hwid_ = hwid;
    cpu_max_ = cpu_max;
    l2cache_ena_ = l2cache_ena;
    plic_irq_max_ = plic_irq_max;
    pslv0 = 0;

    pslv0 = new apb_slv("pslv0",
                         async_reset,
                         VENDOR_OPTIMITECH,
                         OPTIMITECH_PNP);
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
    for (int i = 0; i < SOC_PNP_TOTAL; i++) {
        sensitive << i_cfg[i];
    }
    sensitive << i_apbi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << r.fw_id;
    sensitive << r.idt_l;
    sensitive << r.idt_m;
    sensitive << r.malloc_addr_l;
    sensitive << r.malloc_addr_m;
    sensitive << r.malloc_size_l;
    sensitive << r.malloc_size_m;
    sensitive << r.fwdbg1;
    sensitive << r.fwdbg2;
    sensitive << r.fwdbg3;
    sensitive << r.fwdbg4;
    sensitive << r.fwdbg5;
    sensitive << r.fwdbg6;
    sensitive << r.irq;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

template<int cfg_slots>
apb_pnp<cfg_slots>::~apb_pnp() {
    if (pslv0) {
        delete pslv0;
    }
}

template<int cfg_slots>
void apb_pnp<cfg_slots>::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_apbi, i_apbi.name());
        sc_trace(o_vcd, o_apbo, o_apbo.name());
        sc_trace(o_vcd, o_irq, o_irq.name());
        sc_trace(o_vcd, r.fw_id, pn + ".r.fw_id");
        sc_trace(o_vcd, r.idt_l, pn + ".r.idt_l");
        sc_trace(o_vcd, r.idt_m, pn + ".r.idt_m");
        sc_trace(o_vcd, r.malloc_addr_l, pn + ".r.malloc_addr_l");
        sc_trace(o_vcd, r.malloc_addr_m, pn + ".r.malloc_addr_m");
        sc_trace(o_vcd, r.malloc_size_l, pn + ".r.malloc_size_l");
        sc_trace(o_vcd, r.malloc_size_m, pn + ".r.malloc_size_m");
        sc_trace(o_vcd, r.fwdbg1, pn + ".r.fwdbg1");
        sc_trace(o_vcd, r.fwdbg2, pn + ".r.fwdbg2");
        sc_trace(o_vcd, r.fwdbg3, pn + ".r.fwdbg3");
        sc_trace(o_vcd, r.fwdbg4, pn + ".r.fwdbg4");
        sc_trace(o_vcd, r.fwdbg5, pn + ".r.fwdbg5");
        sc_trace(o_vcd, r.fwdbg6, pn + ".r.fwdbg6");
        sc_trace(o_vcd, r.irq, pn + ".r.irq");
        sc_trace(o_vcd, r.resp_valid, pn + ".r.resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r.resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r.resp_err");
    }

    if (pslv0) {
        pslv0->generateVCD(i_vcd, o_vcd);
    }
}

template<int cfg_slots>
void apb_pnp<cfg_slots>::comb() {
    sc_uint<32> cfgmap[(8 * cfg_slots)];
    sc_uint<32> vrdata;

    v = r;
    for (int i = 0; i < (8 * cfg_slots); i++) {
        cfgmap[i] = 0;
    }
    vrdata = 0;

    v.irq = 0;

    for (int i = 0; i < cfg_slots; i++) {
        cfgmap[(8 * i)] = (0, i_cfg[i].read().descrtype, i_cfg[i].read().descrsize);
        cfgmap[((8 * i) + 1)] = (i_cfg[i].read().vid, i_cfg[i].read().did);
        cfgmap[((8 * i) + 4)] = i_cfg[i].read().addr_start(31, 0);
        cfgmap[((8 * i) + 5)] = i_cfg[i].read().addr_start(63, 32);
        cfgmap[((8 * i) + 6)] = i_cfg[i].read().addr_end(31, 0);
        cfgmap[((8 * i) + 7)] = i_cfg[i].read().addr_end(63, 32);
    }

    if (wb_req_addr.read()(11, 2) == 0) {
        vrdata = hwid_;
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.irq = 1;
        }
    } else if (wb_req_addr.read()(11, 2) == 1) {
        vrdata = r.fw_id.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.fw_id = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 2) {
        vrdata(31, 28) = (cpu_max_ >> 0);
        vrdata[24] = l2cache_ena_;
        vrdata(15, 8) = (cfg_slots >> 0);
        vrdata(7, 0) = (plic_irq_max_ >> 0);
    } else if (wb_req_addr.read()(11, 2) == 3) {
        vrdata = 0;
    } else if (wb_req_addr.read()(11, 2) == 4) {
        vrdata = r.idt_l.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.idt_l = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 5) {
        vrdata = r.idt_m.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.idt_m = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 6) {
        vrdata = r.malloc_addr_l.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.malloc_addr_l = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 7) {
        vrdata = r.malloc_addr_m.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.malloc_addr_m = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 8) {
        vrdata = r.malloc_size_l.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.malloc_size_l = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 9) {
        vrdata = r.malloc_size_m.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.malloc_size_m = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 10) {
        vrdata = r.fwdbg1.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.fwdbg1 = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 11) {
        vrdata = r.fwdbg2.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.fwdbg2 = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 12) {
        vrdata = r.fwdbg3.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.fwdbg3 = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 13) {
        vrdata = r.fwdbg4.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.fwdbg4 = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 14) {
        vrdata = r.fwdbg5.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.fwdbg5 = wb_req_wdata.read();
        }
    } else if (wb_req_addr.read()(11, 2) == 15) {
        vrdata = r.fwdbg6.read();
        if ((w_req_valid.read() & w_req_write.read()) == 1) {
            v.fwdbg6 = wb_req_wdata.read();
        }
    } else if ((wb_req_addr.read()(11, 2) >= 16)
                && (wb_req_addr.read()(11, 2) < (16 + (8 * cfg_slots)))) {
        vrdata = cfgmap[(wb_req_addr.read()(11, 2).to_int() - 16)];
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        apb_pnp_r_reset(v);
    }

    v.resp_valid = w_req_valid.read();
    v.resp_rdata = vrdata;
    v.resp_err = 0;
    o_irq = r.irq.read();
}

template<int cfg_slots>
void apb_pnp<cfg_slots>::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        apb_pnp_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

