/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SystemC CPU wrapper. To interact with the SoC simulator.
 */

#include "api_core.h"
#include "rtl_wrapper.h"

namespace debugger {

RtlWrapper::RtlWrapper(sc_module_name name)
    : sc_module(name),
    o_clk("clk", 1, SC_NS) {

    clockCycles_ = 1000000; // 1 MHz when default resolution = 1 ps

    SC_METHOD(registers);
    sensitive << o_clk.posedge_event();

    SC_METHOD(comb);
    sensitive << r.nrst;
    sensitive << r.resp_mem_data_valid;
    sensitive << r.resp_mem_data;
    sensitive << r.interrupt;

    SC_METHOD(clk_negedge_proc);
    sensitive << o_clk.negedge_event();

    w_nrst = 0;
    v.nrst = 0;
    v.interrupt = false;
    v.resp_mem_data = 0;
    v.resp_mem_data_valid = false;
}

void RtlWrapper::clk_gen() {
    // todo: instead sc_clock
}

void RtlWrapper::comb() {
    o_nrst = r.nrst.read()[1];
    o_resp_mem_data_valid = r.resp_mem_data_valid;
    o_resp_mem_data = r.resp_mem_data;
    o_interrupt = r.interrupt;

    if (!r.nrst.read()[1]) {
    }
}

void RtlWrapper::registers() {
    v.nrst = (r.nrst.read() << 1) | w_nrst;

    r = v;
}

void RtlWrapper::clk_negedge_proc() {
    /** Simulation events queue */
    IFace *cb;
    queue_.initProc();
    queue_.pushPreQueued();
    uint64_t step_cnt = i_timer.read();
    while (cb = queue_.getNext(step_cnt)) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt);
    }

    /** */
    v.resp_mem_data = 0;
    v.resp_mem_data_valid = false;
    if (i_req_mem_valid.read()) {
        uint64_t addr = i_req_mem_addr.read();
        Reg64Type val;
        if (i_req_mem_write.read()) {
            uint8_t strob = i_req_mem_strob.read();
            uint64_t offset = mask2offset(strob);
            int size = mask2size(strob >> offset);

            addr += offset;
            val.val = i_req_mem_data.read();
            ibus_->write(addr, val.buf, size);
            v.resp_mem_data = 0;
        } else {
            ibus_->read(addr, val.buf, sizeof(val));
            v.resp_mem_data = val.val;
        }
        v.resp_mem_data_valid = true;
    }
}

uint64_t RtlWrapper::mask2offset(uint8_t mask) {
    for (int i = 0; i < AXI_DATA_BYTES; i++) {
        if (mask & 0x1) {
            return static_cast<uint64_t>(i);
        }
        mask >>= 1;
    }
    return 0;
}

uint32_t RtlWrapper::mask2size(uint8_t mask) {
    uint32_t bytes = 0;
    for (int i = 0; i < AXI_DATA_BYTES; i++) {
        if (!(mask & 0x1)) {
            break;
        }
        bytes++;
        mask >>= 1;
    }
    return bytes;
}

void RtlWrapper::setClockHz(double hz) {
    sc_time dt = sc_get_time_resolution();
    clockCycles_ = static_cast<int>((1.0 / hz) / dt.to_seconds() + 0.5);
}
    
void RtlWrapper::registerStepCallback(IClockListener *cb, uint64_t t) {
    queue_.put(t, cb);
}

void RtlWrapper::raiseSignal(int idx) {
    switch (idx) {
    case CPU_SIGNAL_RESET:
        w_nrst = 1;
        break;
    case CPU_SIGNAL_EXT_IRQ:
        v.interrupt = true;
        break;
    default:;
    }
}

void RtlWrapper::lowerSignal(int idx) {
    switch (idx) {
    case CPU_SIGNAL_RESET:
        w_nrst = 0;
        break;
    case CPU_SIGNAL_EXT_IRQ:
        v.interrupt = false;
        break;
    default:;
    }
}



}  // namespace debugger

