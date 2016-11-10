/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU synthesizable SystemC class declaration.
 */

#ifndef __DEBUGGER_CPU_RISCV_RTL_H__
#define __DEBUGGER_CPU_RISCV_RTL_H__

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "async_tqueue.h"
#include "coreservices/ithread.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/imemop.h"
#include "coreservices/ibus.h"
#include "coreservices/iclock.h"
#include "rtl_wrapper.h"
#include "riverlib/river_top.h"
#include <systemc.h>

namespace debugger {

class CpuRiscV_RTL : public IService, 
                 public IThread,
                 public ICpuRiscV,
                 public IClock,
                 public IHap {
public:
    CpuRiscV_RTL(const char *name);
    virtual ~CpuRiscV_RTL();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** ICpuRiscV interface */
    virtual void raiseSignal(int idx) { wrapper_->raiseSignal(idx); }
    virtual void lowerSignal(int idx) { wrapper_->lowerSignal(idx); }
    virtual bool isHalt() { return false; }
    virtual void halt() {}
    virtual void go() {}
    virtual void step(uint64_t cnt) {}
    virtual uint64_t getReg(uint64_t idx) { return 0; }
    virtual void setReg(uint64_t idx, uint64_t val) {}
    virtual uint64_t getPC() { return 0; }
    virtual void setPC(uint64_t val) {}
    virtual uint64_t getNPC() { return 0; }
    virtual void setNPC(uint64_t val) {}
    virtual void addBreakpoint(uint64_t addr) {}
    virtual void removeBreakpoint(uint64_t addr) {}
    virtual void hitBreakpoint(uint64_t addr) {}

    /** IClock */
    virtual uint64_t getStepCounter() {
        return wb_step_cnt.read() + 2;
    }

    virtual void registerStepCallback(IClockListener *cb, uint64_t t) {
        wrapper_->registerStepCallback(cb, t);
    }

    virtual uint64_t getClockCounter() {
        return wb_timer.read();
    }

    virtual void registerClockCallback(IClockListener *cb, uint64_t t) {
        wrapper_->registerClockCallback(cb, t);
    }

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    virtual void stop();

protected:
    /** IThread interface */
    virtual void busyLoop();

private:

private:
    AttributeType bus_;
    AttributeType freqHz_;
    event_def config_done_;

    sc_signal<bool> w_clk;
    sc_signal<bool> w_nrst;
    // Timer:
    sc_signal<sc_uint<RISCV_ARCH>> wb_timer;
    sc_signal<sc_uint<64>> wb_step_cnt;
    // Memory interface:
    sc_signal<bool> w_req_mem_valid;
    sc_signal<bool> w_req_mem_write;
    sc_signal<sc_uint<AXI_ADDR_WIDTH>> wb_req_mem_addr;
    sc_signal<sc_uint<AXI_DATA_BYTES>> wb_req_mem_strob;
    sc_signal<sc_uint<AXI_DATA_WIDTH>> wb_req_mem_data;
    sc_signal<bool> w_resp_mem_data_valid;
    sc_signal<sc_uint<AXI_DATA_WIDTH>> wb_resp_mem_data;
    /** Interrupt line from external interrupts controller. */
    sc_signal<bool> w_interrupt;
    // Debug interface

    sc_trace_file *vcd_;
    RiverTop *top_;
    RtlWrapper *wrapper_;
};

DECLARE_CLASS(CpuRiscV_RTL)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_RTL_H__
