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
#include "coreservices/iclklistener.h"

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

    /** ICpuRiscV interface */
    virtual void raiseInterrupt(int idx);
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
    virtual uint64_t getStepCounter() { return 0; }
    virtual void registerStepCallback(IClockListener *cb, uint64_t t);

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:

private:
    AttributeType bus_;
    AttributeType freqHz_;
    event_def config_done_;

    IBus *ibus_;
    AsyncTQueueType queue_;
};

DECLARE_CLASS(CpuRiscV_RTL)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_RTL_H__
