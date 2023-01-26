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

#include "l2_top.h"
#include "api_core.h"

namespace debugger {

L2Top::L2Top(sc_module_name name,
             bool async_reset,
             uint32_t waybits,
             uint32_t ibits)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_l1o("i_l1o", CFG_SLOT_L1_TOTAL),
    o_l1i("o_l1i", CFG_SLOT_L1_TOTAL),
    i_l2i("i_l2i"),
    o_l2o("o_l2o"),
    i_flush_valid("i_flush_valid") {

    async_reset_ = async_reset;
    waybits_ = waybits;
    ibits_ = ibits;
    cache0 = 0;
    amba0 = 0;
    dst0 = 0;

    wb_flush_address = ~0ull;



    dst0 = new L2Destination("dst0", async_reset);
    dst0->i_clk(i_clk);
    dst0->i_nrst(i_nrst);
    dst0->i_resp_valid(w_cache_valid);
    dst0->i_resp_rdata(wb_cache_rdata);
    dst0->i_resp_status(wb_cache_status);
    dst0->i_l1o(i_l1o);
    dst0->o_l1i(o_l1i);
    dst0->i_req_ready(w_req_ready);
    dst0->o_req_valid(w_req_valid);
    dst0->o_req_type(wb_req_type);
    dst0->o_req_addr(wb_req_addr);
    dst0->o_req_size(wb_req_size);
    dst0->o_req_prot(wb_req_prot);
    dst0->o_req_wdata(wb_req_wdata);
    dst0->o_req_wstrb(wb_req_wstrb);


    cache0 = new L2CacheLru("cache0", async_reset,
                             waybits,
                             ibits);
    cache0->i_clk(i_clk);
    cache0->i_nrst(i_nrst);
    cache0->i_req_valid(w_req_valid);
    cache0->i_req_type(wb_req_type);
    cache0->i_req_size(wb_req_size);
    cache0->i_req_prot(wb_req_prot);
    cache0->i_req_addr(wb_req_addr);
    cache0->i_req_wdata(wb_req_wdata);
    cache0->i_req_wstrb(wb_req_wstrb);
    cache0->o_req_ready(w_req_ready);
    cache0->o_resp_valid(w_cache_valid);
    cache0->o_resp_rdata(wb_cache_rdata);
    cache0->o_resp_status(wb_cache_status);
    cache0->i_req_mem_ready(w_req_mem_ready);
    cache0->o_req_mem_valid(w_req_mem_valid);
    cache0->o_req_mem_type(wb_req_mem_type);
    cache0->o_req_mem_size(wb_req_mem_size);
    cache0->o_req_mem_prot(wb_req_mem_prot);
    cache0->o_req_mem_addr(wb_req_mem_addr);
    cache0->o_req_mem_strob(wb_req_mem_strob);
    cache0->o_req_mem_data(wb_req_mem_data);
    cache0->i_mem_data_valid(w_mem_data_valid);
    cache0->i_mem_data(wb_mem_data);
    cache0->i_mem_data_ack(w_mem_data_ack);
    cache0->i_mem_load_fault(w_mem_load_fault);
    cache0->i_mem_store_fault(w_mem_store_fault);
    cache0->i_flush_address(wb_flush_address);
    cache0->i_flush_valid(i_flush_valid);
    cache0->o_flush_end(w_flush_end);


    amba0 = new L2Amba("amba0", async_reset);
    amba0->i_clk(i_clk);
    amba0->i_nrst(i_nrst);
    amba0->o_req_ready(w_req_mem_ready);
    amba0->i_req_valid(w_req_mem_valid);
    amba0->i_req_type(wb_req_mem_type);
    amba0->i_req_size(wb_req_mem_size);
    amba0->i_req_prot(wb_req_mem_prot);
    amba0->i_req_addr(wb_req_mem_addr);
    amba0->i_req_strob(wb_req_mem_strob);
    amba0->i_req_data(wb_req_mem_data);
    amba0->o_resp_data(wb_mem_data);
    amba0->o_resp_valid(w_mem_data_valid);
    amba0->o_resp_ack(w_mem_data_ack);
    amba0->o_resp_load_fault(w_mem_load_fault);
    amba0->o_resp_store_fault(w_mem_store_fault);
    amba0->i_msti(i_l2i);
    amba0->o_msto(o_l2o);


}

L2Top::~L2Top() {
    if (cache0) {
        delete cache0;
    }
    if (amba0) {
        delete amba0;
    }
    if (dst0) {
        delete dst0;
    }
}

void L2Top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_l2i, i_l2i.name());
        sc_trace(o_vcd, o_l2o, o_l2o.name());
        sc_trace(o_vcd, i_flush_valid, i_flush_valid.name());
    }

    if (cache0) {
        cache0->generateVCD(i_vcd, o_vcd);
    }
    if (amba0) {
        amba0->generateVCD(i_vcd, o_vcd);
    }
    if (dst0) {
        dst0->generateVCD(i_vcd, o_vcd);
    }
}

}  // namespace debugger

