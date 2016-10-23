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
    o_clk("clk", 1, SC_NS),
    r_nrst("nrst", false) {

    clockCycles_ = 1000000; // 1 MHz when default resolution = 1 ps

    SC_METHOD(clk_posedge_proc);
    sensitive << o_clk.posedge_event();

    SC_METHOD(clk_negedge_proc);
    sensitive << o_clk.negedge_event();

    w_nrst = false;
    w_interrupt = false;
    v.resp_mem_data = 0;
    v.resp_mem_ready = false;
}

void RtlWrapper::clk_gen() {
    // todo: instead sc_clock
}

void RtlWrapper::clk_posedge_proc() {
    /** Handle signals written out of context of current thread: */
    r_nrst.write(w_nrst);
    r_interrupt.write(w_interrupt);
    r.resp_mem_data.write(v.resp_mem_data);
    r.resp_mem_ready.write(v.resp_mem_ready);

    /** Simulation events queue */
    IFace *cb;
    queue_.initProc();
    queue_.pushPreQueued();
    uint64_t step_cnt = i_timer.read();
    while (cb = queue_.getNext(step_cnt)) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt);
    }

    o_nrst.write(r_nrst);
    o_resp_mem_ready.write(r.resp_mem_ready);
    o_resp_mem_data.write(r.resp_mem_data);
    o_interrupt.write(r_interrupt);
}

void RtlWrapper::clk_negedge_proc() {
    /** */
    v.resp_mem_data = 0;
    v.resp_mem_ready = false;
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
        v.resp_mem_ready = true;
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
        w_nrst = true;
        break;
    case CPU_SIGNAL_EXT_IRQ:
        w_interrupt = true;
        break;
    default:;
    }
}

void RtlWrapper::lowerSignal(int idx) {
    switch (idx) {
    case CPU_SIGNAL_RESET:
        w_nrst = false;
        break;
    case CPU_SIGNAL_EXT_IRQ:
        w_interrupt = false;
        break;
    default:;
    }
}



}  // namespace debugger

