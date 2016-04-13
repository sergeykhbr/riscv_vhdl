/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU functional simlator class definition.
 */

#ifndef __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__
#define __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/ithread.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/imemop.h"
#include "coreservices/ihostio.h"
#include "coreservices/iclock.h"
#include "coreservices/iclklistener.h"
#include "instructions.h"

namespace debugger {

class CpuRiscV_Functional : public IService, 
                 public IThread,
                 public ICpuRiscV,
                 public IHostIO,
                 public IClock {
public:
    CpuRiscV_Functional(const char *name);
    ~CpuRiscV_Functional();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** ICpuRiscV interface */
    virtual void halt() {}

    /** IHostIO */
    virtual uint64_t write(uint16_t adr, uint64_t val);
    virtual uint64_t read(uint16_t adr, uint64_t *val);


    /** IClock */
    virtual uint64_t getStepCounter() { return cpu_data_.step_cnt; }
    virtual void registerStepCallback(IClockListener *cb, uint64_t t);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    void reset();
    void handleInterrupt();
    void fetchInstruction();
    IInstruction *decodeInstruction(uint32_t *rpayload);
    void executeInstruction(IInstruction *instr, uint32_t *rpayload);

    void stepUpdate();
    void queueUpdate();

private:
    AttributeType parentThread_;
    AttributeType phys_mem_;
    AttributeType listExtISA_;
    AttributeType freqHz_;

    Axi4TransactionType memop_;

    enum QueueItemNames {
        Queue_Time, 
        Queue_IFace, 
        Queue_Total
    };
    AttributeType stepQueue_;
    unsigned stepQueue_len_;    // to avoid reallocation

    // Registers:
    AttributeType listInstr_;
    CpuDataType cpu_data_;
};

DECLARE_CLASS(CpuRiscV_Functional)

}  // namespace debugger

#endif  // __DEBUGGER_CPU_RISCV_FUNCTIONAL_H__
