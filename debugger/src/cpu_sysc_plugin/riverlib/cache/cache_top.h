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
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset active LOW
    // Control path:
    sc_in<bool> i_req_ctrl_valid;                       // Control request from CPU Core is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_ctrl_addr;     // Control request address
    sc_out<bool> o_req_ctrl_ready;                      // Control request from CPU Core is accepted
    sc_out<bool> o_resp_ctrl_valid;                     // ICache response is valid and can be accepted
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_resp_ctrl_addr;   // ICache response address
    sc_out<sc_uint<32>> o_resp_ctrl_data;               // ICache read data
    sc_in<bool> i_resp_ctrl_ready;                      // CPU Core is ready to accept ICache response
    // Data path:
    sc_in<bool> i_req_data_valid;                       // Data path request from CPU Core is valid
    sc_in<bool> i_req_data_write;                       // Data write memopy operation flag
    sc_in<sc_uint<2>> i_req_data_size;                  // Memory operation size: 0=1B; 1=2B; 2=4B; 3=8B
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_data_addr;     // Memory operation address
    sc_in<sc_uint<RISCV_ARCH>> i_req_data_data;         // Memory operation write value
    sc_out<bool> o_req_data_ready;                      // Memory operation request accepted by DCache
    sc_out<bool> o_resp_data_valid;                     // DCache response is ready
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_resp_data_addr;   // DCache response address
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data_data;       // DCache response read data
    sc_in<bool> i_resp_data_ready;                      // CPU Core is ready to accept DCache repsonse
    // Memory interface:
    sc_in<bool> i_req_mem_ready;                       // System Bus (AXI) is available
    sc_out<bool> o_req_mem_valid;                       // Memory operation to system bus is valid
    sc_out<bool> o_req_mem_write;                       // Memory operation write flag
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_mem_addr;     // Requesting address
    sc_out<sc_uint<BUS_DATA_BYTES>> o_req_mem_strob;    // Writing strob 1 bit per 1 byte (AXI compliance)
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_req_mem_data;     // Writing value
    sc_in<bool> i_resp_mem_data_valid;                  // Memory operation from system bus is completed
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_resp_mem_data;     // Read value


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
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_ctrl_req_mem_wdata;
    sc_signal<bool> w_ctrl_resp_mem_data_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_ctrl_resp_mem_data;
    sc_signal<bool> w_ctrl_ready;
    // Memory Data interface:
    sc_signal<bool> w_data_req_mem_valid;
    sc_signal<bool> w_data_req_mem_write;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_data_req_mem_addr;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_data_req_mem_strob;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_data_req_mem_wdata;
    sc_signal<bool> w_data_resp_mem_data_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_data_resp_mem_data;
    sc_signal<bool> w_data_ready;

    ICache *i0;
    DCache *d0;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TOP_H__
