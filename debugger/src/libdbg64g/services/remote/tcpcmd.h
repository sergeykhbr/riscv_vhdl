/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      TCP commands parser/processor.
 */

#ifndef __DEBUGGER_TCPCMD_H__
#define __DEBUGGER_TCPCMD_H__

#include <api_core.h>
#include <iclass.h>
#include <iservice.h>
#include <ihap.h>
#include "coreservices/ilink.h"
#include "coreservices/ithread.h"
#include "coreservices/icpugen.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/icmdexec.h"
#include "coreservices/isrccode.h"
#include "coreservices/iclock.h"
#include "coreservices/iwire.h"
#include "coreservices/irawlistener.h"

namespace debugger {

class TcpCommands : public IRawListener,
                    public IHap,
                    public IClockListener {
 public:
    explicit TcpCommands(IService *parent);
    ~TcpCommands();

    /** IRawListener interface */
    virtual void updateData(const char *buf, int buflen);

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    /** IClockListener */
    virtual void stepCallback(uint64_t t);

    /** Common acccess methods */
    AttributeType *response();

 protected:
    IFace *getInterface(const char *name) {
        return parent_->getInterface(name);
    }

 private:
    void processCommand();
    void br_add(const AttributeType &symb, AttributeType *res);
    void br_rm(const AttributeType &symb, AttributeType *res);
    void go_msec(const AttributeType &symb, AttributeType *res);
    void go_until(const AttributeType &symb, AttributeType *res);
    void step(int cnt, AttributeType *res);
    void symb2addr(const char *symbol, AttributeType *res);
    void power_on(AttributeType *res);
    void power_off(AttributeType *res);

 private:
    char rxbuf_[4096];
    int rxcnt_;
    AttributeType cpu_;
    AttributeType executor_;
    AttributeType source_;

    AttributeType resp_;

    IService *parent_;
    ICmdExecutor *iexec_;
    ISourceCode *isrc_;
    ICpuRiscV *iriscv_;
    IClock *iclk_;
    AttributeType *cpuLogLevel_;

    event_def eventHalt_;
    event_def eventDelayMs_;
    event_def eventPowerChanged_;
};

}  // namespace debugger

#endif  // __DEBUGGER_TCPCMD_H__
