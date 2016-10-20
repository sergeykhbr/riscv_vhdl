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
}

CpuRiscV_RTL::~CpuRiscV_RTL() {
    RISCV_event_close(&config_done_);
}

void CpuRiscV_RTL::postinitService() {
    ibus_ = static_cast<IBus *>(
       RISCV_get_service_iface(bus_.to_string(), IFACE_BUS));

    if (!ibus_) {
        RISCV_error("Bus interface '%s' not found", 
                    bus_.to_string());
        return;
    }
}

void CpuRiscV_RTL::hapTriggered(IFace *isrc, EHapType type,
                                const char *descr) {
    RISCV_event_set(&config_done_);
}

void CpuRiscV_RTL::busyLoop() {
    RISCV_event_wait(&config_done_);

    IFace *cb;
    uint64_t cur_time = 0;
    while (isEnabled()) {
        queue_.initProc();
        queue_.pushPreQueued();
        
        while (cb = queue_.getNext(cur_time)) {
            
            /** 
             * We check pre-queued events to provide possiblity of new events
             * on the same step.
             */
            queue_.pushPreQueued();
        }
        cur_time++;
    }
}

void CpuRiscV_RTL::raiseInterrupt(int idx) {
}

void CpuRiscV_RTL::registerStepCallback(IClockListener *cb,
                                               uint64_t t) {
    queue_.put(t, cb);
}

}  // namespace debugger

