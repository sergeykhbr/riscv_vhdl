/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include <api_core.h>
#include <iclass.h>
#include <iservice.h>
#include "coreservices/ireset.h"
#include "coreservices/idmi.h"
#include "coreservices/idport.h"
#include "coreservices/imemop.h"
#include "coreservices/ijtag.h"

namespace debugger {

class DmiFunctional : public IService,
                      public IMemoryOperation,
                      public IDmi {
 public:
    explicit DmiFunctional(const char *name);
    virtual ~DmiFunctional();

    /** IService interface */
    virtual void postinitService() override;

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** IDmi interface */
    virtual void dtm_dmihardreset() {}
    virtual void dmi_read(uint32_t addr, uint32_t *rdata);
    virtual void dmi_write(uint32_t addr, uint32_t wdata);
    virtual EDmistatus dmi_status() { return DMI_STAT_SUCCESS; }

 private:
    void executeCommand();
    void setCmdErr(uint32_t v) {
        cmderr_ = v;
    }
    uint32_t getCmdErr() { return cmderr_; }

 private:
    AttributeType sysbus_;
    AttributeType busid_;
    AttributeType cpumax_;
    AttributeType dataregTotal_;
    AttributeType progbufTotal_;
    AttributeType hartlist_;

    IMemoryOperation *ibus_;

    struct HartDataType {
        IDPort *idport; // if 0, hart is not available
    } *phartdata_;

    uint32_t ndmreset_; // Reset signal from DM to the rest of the hardware. Default=0, Active=1
    uint32_t hartsel_;
    Reg64Type arg0_;
    Reg64Type arg1_;
    Reg64Type arg2_;
    IJtag::dmi_command_type command_;
    uint32_t autoexecdata_;
    uint32_t autoexecprogbuf_;
    uint32_t progbuf_[32];
    uint32_t cmderr_;
};

DECLARE_CLASS(DmiFunctional)

}  // namespace debugger

