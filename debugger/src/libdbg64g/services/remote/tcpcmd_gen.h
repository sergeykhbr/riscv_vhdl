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

#ifndef __DEBUGGER_TCPCMD_GEN_H__
#define __DEBUGGER_TCPCMD_GEN_H__

#include <api_core.h>
#include <iclass.h>
#include <iservice.h>
#include <ihap.h>
#include "coreservices/ilink.h"
#include "coreservices/ithread.h"
#include "coreservices/icpufunctional.h"
#include "coreservices/icmdexec.h"
#include "coreservices/isrccode.h"
#include "coreservices/ikeyboard.h"
#include "coreservices/iclock.h"
#include "coreservices/imotor.h"
#include "coreservices/isensor.h"
#include "coreservices/iwire.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iserial.h"
#include "coreservices/idisplay.h"
#include "igui.h"

namespace debugger {

class TcpCommandsGen : public IRawListener,
                       public IHap,
                       public IClockListener {
 public:
    explicit TcpCommandsGen(IService *parent);
    virtual ~TcpCommandsGen();

    /** IRawListener interface */
    virtual int updateData(const char *buf, int buflen);

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

    /** IClockListener */
    virtual void stepCallback(uint64_t t);

    /** Common acccess methods */
    void setPlatformConfig(AttributeType *cfg);
    uint8_t *response_buf() { return reinterpret_cast<uint8_t *>(respbuf_); }
    int response_size() { return respcnt_; }
    void done() { respcnt_ = 0; }

 protected:
    virtual int processCommand(const char *cmdbuf, int bufsz) = 0;
    virtual bool isStartMarker(char s) = 0;
    virtual bool isEndMarker(const char *s, int sz) = 0;

    IFace *getInterface(const char *name) {
        return parent_->getInterface(name);
    }

    void br_add(const AttributeType &symb, AttributeType *res);
    void br_rm(const AttributeType &symb, AttributeType *res);
    void go_msec(const AttributeType &symb, AttributeType *res);
    void go_until(const AttributeType &symb, AttributeType *res);
    void step(int cnt, AttributeType *res);
    void symb2addr(const char *symbol, AttributeType *res);
    void power_on(const char *btn_name, AttributeType *res);
    void power_off(const char *btn_name, AttributeType *res);

 protected:
    char rxbuf_[4096];
    int rxcnt_;
    AttributeType platformConfig_;
    AttributeType cpu_;
    AttributeType executor_;
    AttributeType source_;
    AttributeType gui_;

    char *respbuf_;
    int resptotal_;
    int respcnt_;

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

    enum EState {
        State_Idle,
        State_Started,
        State_Ready
    } estate_;
};

}  // namespace debugger

#endif  // __DEBUGGER_TCPCMD_GEN_H__
