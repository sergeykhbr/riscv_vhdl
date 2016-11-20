/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU synthesizable SystemC class declaration.
 */

#include "api_core.h"
#include "cpu_riscv_rtl.h"

namespace debugger {

CpuRiscV_RTL::CpuRiscV_RTL(const char *name)  
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("Bus", &bus_);
    registerAttribute("FreqHz", &freqHz_);

    bus_.make_string("");
    freqHz_.make_uint64(1);
    RISCV_event_create(&config_done_, "config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
    sc_set_default_time_unit(1, SC_NS);
#if GENERATE_VCD
    vcd_ = sc_create_vcd_trace_file("river_sim");
    vcd_->set_time_unit(1, SC_PS);
#else
    vcd_ = 0;
#endif
    /** Create all objects, then initilize SystemC context: */
    wrapper_ = new RtlWrapper("wrapper");
    w_clk = wrapper_->o_clk;
    wrapper_->o_nrst(w_nrst);
    wrapper_->i_timer(wb_timer);
    wrapper_->i_step_cnt(wb_step_cnt);
    wrapper_->i_req_mem_valid(w_req_mem_valid);
    wrapper_->i_req_mem_write(w_req_mem_write);
    wrapper_->i_req_mem_addr(wb_req_mem_addr);
    wrapper_->i_req_mem_strob(wb_req_mem_strob);
    wrapper_->i_req_mem_data(wb_req_mem_data);
    wrapper_->o_resp_mem_data_valid(w_resp_mem_data_valid);
    wrapper_->o_resp_mem_data(wb_resp_mem_data);
    wrapper_->o_interrupt(w_interrupt);

    top_ = new RiverTop("top", vcd_);
    top_->i_clk(wrapper_->o_clk);
    top_->i_nrst(w_nrst);
    top_->o_timer(wb_timer);
    top_->o_req_mem_valid(w_req_mem_valid);
    top_->o_req_mem_write(w_req_mem_write);
    top_->o_req_mem_addr(wb_req_mem_addr);
    top_->o_req_mem_strob(wb_req_mem_strob);
    top_->o_req_mem_data(wb_req_mem_data);
    top_->i_resp_mem_data_valid(w_resp_mem_data_valid);
    top_->i_resp_mem_data(wb_resp_mem_data);
    top_->i_ext_irq(w_interrupt);
    top_->o_step_cnt(wb_step_cnt);

    //sc_trace(vcd_, w_req_mem_valid, "w_req_mem_valid");
    //sc_trace(vcd_, wb_req_mem_addr, "wb_req_mem_addr");
    //sc_trace(vcd_, w_resp_mem_ready, "w_resp_mem_ready");
    //sc_trace(vcd_, wb_resp_mem_data, "wb_resp_mem_data");

    sc_start(0, SC_NS);
}

CpuRiscV_RTL::~CpuRiscV_RTL() {
    RISCV_event_close(&config_done_);
}

void CpuRiscV_RTL::postinitService() {
    IBus *ibus = static_cast<IBus *>(
       RISCV_get_service_iface(bus_.to_string(), IFACE_BUS));

    if (!ibus) {
        RISCV_error("Bus interface '%s' not found", 
                    bus_.to_string());
        return;
    }
    wrapper_->setBus(ibus);

    if (!run()) {
        RISCV_error("Can't create thread.", NULL);
        return;
    }

    wrapper_->setClockHz(freqHz_.to_int());
}

void CpuRiscV_RTL::predeleteService() {
    stop();
}

void CpuRiscV_RTL::hapTriggered(IFace *isrc, EHapType type,
                                const char *descr) {
    RISCV_event_set(&config_done_);
}

void CpuRiscV_RTL::stop() {
    sc_stop();
    if (vcd_) {
        //sc_close_vcd_trace_file(vcd_);
    }

    IThread::stop();
}

void CpuRiscV_RTL::busyLoop() {
    RISCV_event_wait(&config_done_);

    sc_start();
}

}  // namespace debugger

