/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SystemC CPU wrapper. To interact with the SoC simulator. */

#ifndef __DEBUGGER_RTL_WRAPPER_H__
#define __DEBUGGER_RTL_WRAPPER_H__

#include "async_tqueue.h"
#include "coreservices/ibus.h"
#include "coreservices/iclklistener.h"
#include "core/river_cfg.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(RtlWrapper) {
    sc_clock o_clk;
    sc_out<bool> o_nrst;
    // Timer:
    sc_in<sc_uint<RISCV_ARCH>> i_timer;
    // Memory interface:
    sc_in<bool> i_req_mem_valid;
    sc_in<bool> i_req_mem_write;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_req_mem_addr;
    sc_in<sc_uint<AXI_DATA_BYTES>> i_req_mem_strob;
    sc_in<sc_uint<AXI_DATA_WIDTH>> i_req_mem_data;
    sc_out<bool> o_resp_mem_ready;
    sc_out<sc_uint<AXI_DATA_WIDTH>> o_resp_mem_data;

    sc_signal<bool> r_nrst;
    bool w_nrst;

    sc_signal<sc_uint<AXI_DATA_WIDTH>> wb_rd_value;
    sc_signal<sc_uint<AXI_DATA_WIDTH>> rb_rd_value;
    sc_signal<bool> w_resp_ready;
    sc_signal<bool> r_resp_ready;

    void clk_proc();
    void mem_access();

    SC_HAS_PROCESS(RtlWrapper);

    RtlWrapper(sc_module_name name);

public:
    void setBus(IBus *v) { ibus_ = v; }
    void setReset(bool v) { w_nrst = v; }
    void registerStepCallback(IClockListener *cb, uint64_t t) {
        queue_.put(t, cb);
    }

private:
    IBus *ibus_;
    AsyncTQueueType queue_;

};

}  // namespace debugger

#endif  // __DEBUGGER_RTL_WRAPPER_H__
