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

    SC_METHOD(clk_proc);
    sensitive << o_clk.posedge_event();

    SC_METHOD(mem_access);
    sensitive << o_clk.negedge_event();

    w_nrst = false;
    wb_rd_value = 0;
    w_resp_ready = 0;
}

void RtlWrapper::clk_proc() {
    /** Latch external reset */
    r_nrst.write(w_nrst);
    rb_rd_value.write(wb_rd_value);
    r_resp_ready.write(w_resp_ready);

    /** Simulation events queue */
    IFace *cb;
    queue_.initProc();
    queue_.pushPreQueued();
    uint64_t step_cnt = i_timer.read();
    while (cb = queue_.getNext(step_cnt)) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt);
    }

    o_nrst.write(r_nrst);
    o_resp_mem_ready.write(r_resp_ready);
}

void RtlWrapper::mem_access() {
    /** */
    wb_rd_value = 0;
    w_resp_ready = 0;
    if (i_req_mem_valid.read()) {
        uint64_t addr = i_req_mem_addr.read();
        Reg64Type val;
        ibus_->read(addr, val.buf, sizeof(val));
        wb_rd_value = val.val;
        w_resp_ready = true;
    }
}

}  // namespace debugger

