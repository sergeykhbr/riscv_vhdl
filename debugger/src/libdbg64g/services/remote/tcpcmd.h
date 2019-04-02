/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_TCPCMD_H__
#define __DEBUGGER_TCPCMD_H__

#include <api_core.h>
#include <iclass.h>
#include <iservice.h>
#include <ihap.h>
#include "coreservices/ilink.h"
#include "coreservices/ithread.h"
#include "coreservices/icpufunctional.h"
#include "coreservices/icmdexec.h"
#include "coreservices/isrccode.h"
#include "coreservices/iclock.h"
#include "coreservices/iwire.h"
#include "coreservices/irawlistener.h"
#include "igui.h"

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
    AttributeType gui_;

    AttributeType resp_;

    IService *parent_;
    ICmdExecutor *iexec_;
    ISourceCode *isrc_;
    ICpuFunctional *icpufunc_;
    IClock *iclk_;
    IGui *igui_;
    AttributeType *cpuLogLevel_;

    event_def eventHalt_;
    event_def eventDelayMs_;
    event_def eventPowerChanged_;
};

}  // namespace debugger

#endif  // __DEBUGGER_TCPCMD_H__
