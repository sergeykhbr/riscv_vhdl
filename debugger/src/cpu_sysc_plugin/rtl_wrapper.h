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
    sc_in<sc_uint<64>> i_step_cnt;
    // Memory interface:
    sc_out<bool> o_req_mem_ready;
    sc_in<bool> i_req_mem_valid;
    sc_in<bool> i_req_mem_write;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_mem_addr;
    sc_in<sc_uint<BUS_DATA_BYTES>> i_req_mem_strob;
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_req_mem_data;
    sc_out<bool> o_resp_mem_data_valid;
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_resp_mem_data;
    /** Interrupt line from external interrupts controller. */
    sc_out<bool> o_interrupt;


    struct RegistersType {
        sc_signal<sc_uint<BUS_DATA_WIDTH>> resp_mem_data;
        sc_signal<bool> resp_mem_data_valid;
        sc_signal<sc_uint<3>> wait_state_cnt;
        sc_signal<sc_bv<5>> nrst;
        sc_signal<bool> interrupt;
    } r, v;
    bool w_nrst;
    bool w_interrupt;

    void clk_gen();
    void comb();
    void registers();
    void clk_negedge_proc();

    SC_HAS_PROCESS(RtlWrapper);

    RtlWrapper(sc_module_name name);

public:
    void setBus(IBus *v) { ibus_ = v; }

    /** Default time resolution 1 picosecond. */
    void setClockHz(double hz);
   
    void registerStepCallback(IClockListener *cb, uint64_t t);
    void registerClockCallback(IClockListener *cb, uint64_t t);
    void raiseSignal(int idx);
    void lowerSignal(int idx);

private:
    uint64_t mask2offset(uint8_t mask);
    uint32_t mask2size(uint8_t mask);       // nask with removed offset

private:
    IBus *ibus_;
    int clockCycles_;   // default in [ps]
    AsyncTQueueType step_queue_;
    AsyncTQueueType clock_queue_;
    uint64_t step_cnt_z;
};

}  // namespace debugger

#endif  // __DEBUGGER_RTL_WRAPPER_H__
