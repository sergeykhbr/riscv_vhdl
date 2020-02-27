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

#include "l2_top.h"

namespace debugger {

L2Top::L2Top(sc_module_name name, bool async_reset) : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_l1o0("i_l1o0"),
    o_l1i0("o_l1i0"),
    i_l1o1("i_l1o1"),
    o_l1i1("o_l1i1"),
    i_l1o2("i_l1o2"),
    o_l1i2("o_l1i2"),
    i_l1o3("i_l1o3"),
    o_l1i3("o_l1i3"),
    i_acpo("i_acpo"),
    o_acpi("o_acpi"),
    i_msti("i_msti"),
    o_msto("o_msto") {

    async_reset_ = async_reset;

    dst_ = new L2Destination("dst0", async_reset);
    dst_->i_clk(i_clk);
    dst_->i_nrst(i_nrst);
    dst_->i_resp_valid(w_cache_valid);
    dst_->i_resp_rdata(wb_cache_rdata);
    dst_->i_resp_status(wb_cache_status);
    dst_->i_l1o0(i_l1o0);
    dst_->o_l1i0(o_l1i0);
    dst_->i_l1o1(i_l1o1);
    dst_->o_l1i1(o_l1i1);
    dst_->i_l1o2(i_l1o2);
    dst_->o_l1i2(o_l1i2);
    dst_->i_l1o3(i_l1o3);
    dst_->o_l1i3(o_l1i3);
    dst_->i_acpo(i_acpo);
    dst_->o_acpi(o_acpi);
    dst_->i_req_ready(w_req_ready);
    dst_->o_req_valid(w_req_valid);
    dst_->o_req_type(wb_req_type);
    dst_->o_req_addr(wb_req_addr);
    dst_->o_req_size(wb_req_size);
    dst_->o_req_prot(wb_req_prot);
    dst_->o_req_wdata(wb_req_wdata);
    dst_->o_req_wstrb(wb_req_wstrb);

    cache_ = new L2CacheLru("cache0", async_reset);
    cache_->i_clk(i_clk);
    cache_->i_nrst(i_nrst);
    cache_->i_req_valid(w_req_valid);
    cache_->i_req_type(wb_req_type);
    cache_->i_req_size(wb_req_size);
    cache_->i_req_prot(wb_req_prot);
    cache_->i_req_addr(wb_req_addr);
    cache_->i_req_wdata(wb_req_wdata);
    cache_->i_req_wstrb(wb_req_wstrb);
    cache_->o_req_ready(w_req_ready);
    cache_->o_resp_valid(w_cache_valid);
    cache_->o_resp_rdata(wb_cache_rdata);
    cache_->o_resp_status(wb_cache_status);
    cache_->i_req_mem_ready(w_req_mem_ready);
    cache_->o_req_mem_valid(w_req_mem_valid);
    cache_->o_req_mem_type(wb_req_mem_type);
    cache_->o_req_mem_size(wb_req_mem_size);
    cache_->o_req_mem_prot(wb_req_mem_prot);
    cache_->o_req_mem_addr(wb_req_mem_addr);
    cache_->o_req_mem_strob(wb_req_mem_strob),
    cache_->o_req_mem_data(wb_req_mem_data);
    cache_->i_mem_data_valid(w_mem_data_valid);
    cache_->i_mem_data(wb_mem_data);
    cache_->i_mem_data_ack(w_mem_data_ack);
    cache_->i_mem_load_fault(w_mem_load_fault);
    cache_->i_mem_store_fault(w_mem_store_fault);
    cache_->i_flush_address(wb_flush_address);
    cache_->i_flush_valid(w_flush_valid);
    cache_->o_flush_end(w_flush_end);

    wb_flush_address = 0;
    w_flush_end = 0;

    amba_ = new L2Amba("amba0", async_reset);
    amba_->i_clk(i_clk);
    amba_->i_nrst(i_nrst);
    amba_->o_req_ready(w_req_mem_ready);
    amba_->i_req_valid(w_req_mem_valid);
    amba_->i_req_type(wb_req_mem_type);
    amba_->i_req_size(wb_req_mem_size);
    amba_->i_req_prot(wb_req_mem_prot);
    amba_->i_req_addr(wb_req_mem_addr);
    amba_->i_req_strob(wb_req_mem_strob);
    amba_->i_req_data(wb_req_mem_data);
    amba_->o_resp_data(wb_mem_data);
    amba_->o_resp_valid(w_mem_data_valid);
    amba_->o_resp_ack(w_mem_data_ack);
    amba_->o_resp_load_fault(w_mem_load_fault);
    amba_->o_resp_store_fault(w_mem_store_fault);
    amba_->i_msti(l2i);
    amba_->o_msto(l2o);

    serdes_ = new L2SerDes("serdes0", async_reset);
    serdes_->i_clk(i_clk);
    serdes_->i_nrst(i_nrst);
    serdes_->o_l2i(l2i);
    serdes_->i_l2o(l2o);
    serdes_->i_msti(i_msti);
    serdes_->o_msto(o_msto);
}

L2Top::~L2Top() {
    delete cache_;
    delete amba_;
    delete serdes_;
    delete dst_;
}

void L2Top::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_msto, o_msto.name());
        sc_trace(o_vcd, i_msti, i_msti.name());

        std::string pn(name());
        sc_trace(o_vcd, w_cache_valid, pn + ".w_cache_valid");
        sc_trace(o_vcd, wb_cache_rdata, pn + ".wb_cache_rdata");
        sc_trace(o_vcd, wb_cache_status, pn + ".wb_cache_status");
    }
    cache_->generateVCD(i_vcd, o_vcd);
    amba_->generateVCD(i_vcd, o_vcd);
    serdes_->generateVCD(i_vcd, o_vcd);
    dst_->generateVCD(i_vcd, o_vcd);
}

}  // namespace debugger
