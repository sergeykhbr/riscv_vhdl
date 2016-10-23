/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SystemC CPU wrapper. To interact with the SoC simulator. */

#ifndef __DEBUGGER_RTL_WRAPPER_H__
#define __DEBUGGER_RTL_WRAPPER_H__

#include "async_tqueue.h"
#include "coreservices/ibus.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/iclklistener.h"
#include "riverlib/river_cfg.h"
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
    /** Interrupt line from external interrupts controller. */
    sc_out<bool> o_interrupt;

    sc_signal<bool> r_nrst;
    bool w_nrst;

    sc_signal<bool> r_interrupt;
    bool w_interrupt;

    struct RegistersType {
        sc_signal<sc_uint<AXI_DATA_WIDTH>> resp_mem_data;
        sc_signal<bool> resp_mem_ready;
    } r, v;

    void clk_gen();
    void clk_posedge_proc();
    void clk_negedge_proc();

    SC_HAS_PROCESS(RtlWrapper);

    RtlWrapper(sc_module_name name);

public:
    void setBus(IBus *v) { ibus_ = v; }

    /** Default time resolution 1 picosecond. */
    void setClockHz(double hz);
   
    void registerStepCallback(IClockListener *cb, uint64_t t);
    void raiseSignal(int idx);
    void lowerSignal(int idx);

private:
    uint64_t mask2offset(uint8_t mask);
    uint32_t mask2size(uint8_t mask);       // nask with removed offset

private:
    IBus *ibus_;
    int clockCycles_;   // default in [ps]
    AsyncTQueueType queue_;

};

}  // namespace debugger

#endif  // __DEBUGGER_RTL_WRAPPER_H__
