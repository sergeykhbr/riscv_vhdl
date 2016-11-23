/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Cache Top level.
 */

#ifndef __DEBUGGER_RIVERLIB_CACHE_TOP_H__
#define __DEBUGGER_RIVERLIB_CACHE_TOP_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "icache.h"
#include "dcache.h"

namespace debugger {

SC_MODULE(CacheTop) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Control path:
    sc_in<bool> i_req_ctrl_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_ctrl_addr;
    sc_out<bool> o_resp_ctrl_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_resp_ctrl_addr;
    sc_out<sc_uint<32>> o_resp_ctrl_data;
    // Data path:
    sc_in<bool> i_req_data_valid;
    sc_in<bool> i_req_data_write;
    sc_in<sc_uint<2>> i_req_data_size; // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_data_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_req_data_data;
    sc_out<bool> o_resp_data_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_resp_data_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data_data;
    // Memory interface:
    sc_out<bool> o_req_mem_valid;
    sc_out<bool> o_req_mem_write;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_mem_addr;
    sc_out<sc_uint<BUS_DATA_BYTES>> o_req_mem_strob;
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_req_mem_data;
    sc_in<bool> i_resp_mem_data_valid;
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_resp_mem_data;

    sc_out<bool> o_hold;


    void comb();
    void registers();

    SC_HAS_PROCESS(CacheTop);

    CacheTop(sc_module_name name_, sc_trace_file *vcd=0);
    virtual ~CacheTop();

private:
    static const uint8_t State_Idle = 0;
    static const uint8_t State_IMem = 1;
    static const uint8_t State_DMem = 2;

    struct RegistersType {
        sc_signal<sc_uint<2>> state;
    } v, r;

    // Memory Control interface:
    sc_signal<bool> w_ctrl_req_mem_valid;
    sc_signal<bool> w_ctrl_req_mem_write;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_ctrl_req_mem_addr;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_ctrl_req_mem_strob;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_ctrl_req_mem_data;
    sc_signal<bool> w_ctrl_resp_mem_data_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_ctrl_resp_mem_data;
    // Memory Data interface:
    sc_signal<bool> w_data_req_mem_valid;
    sc_signal<bool> w_data_req_mem_write;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_data_req_mem_addr;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_data_req_mem_strob;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_data_req_mem_data;
    sc_signal<bool> w_data_resp_mem_data_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_data_resp_mem_data;

    bool w_mem_valid;
    bool w_mem_write;
    sc_uint<BUS_ADDR_WIDTH> wb_mem_addr;
    sc_uint<BUS_DATA_BYTES> wb_mem_strob;
    sc_uint<BUS_DATA_WIDTH> wb_mem_wdata;

    ICache *i0;
    DCache *d0;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TOP_H__
