/**
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

#include <iclass.h>
#include <iservice.h>
#include "coreservices/imemop.h"
#include "coreservices/iirq.h"
#include "coreservices/iclock.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

static const int CLINT_HART_MAX = 4096;

class CLINT : public RegMemBankGeneric,
              public IIrqController {
 public:
    explicit CLINT(const char *name);

    /** IService interface */
    virtual void postinitService() override;

    /** IIrqController */
    virtual int requestInterrupt(IFace *isrc, int idx) { return 0; }
    virtual int getPendingRequest(int ctxid);

 private:
    void setTimer(uint64_t v);
    void updateTimer();

 private:

    class CLINT_MSIP_TYPE : public GenericReg32Bank {
     public:
        CLINT_MSIP_TYPE(IService *parent, const char *name, uint64_t addr)
            : GenericReg32Bank(parent, name, addr, CLINT_HART_MAX) {}
    };

    class CLINT_MTIMECMP_TYPE : public GenericReg64Bank {
     public:
        CLINT_MTIMECMP_TYPE(IService *parent, const char *name, uint64_t addr)
            : GenericReg64Bank(parent, name, addr, CLINT_HART_MAX - 1) {
            // shouldn't be reset on reset signal
        }
    };

    class CLINT_MTIME_TYPE : public MappedReg64Type {
     public:
        CLINT_MTIME_TYPE(IService *parent, const char *name, uint64_t addr)
            : MappedReg64Type(parent, name, addr) {}

     protected:
        virtual uint64_t aboutToRead(uint64_t cur_val) override;
        virtual uint64_t aboutToWrite(uint64_t new_val) override;
    };

    AttributeType clock_;

    IClock *iclk_;

    CLINT_MSIP_TYPE msip;            // [000000..003fff] 1 register (8-Bytes) per hart
    CLINT_MTIMECMP_TYPE mtimecmp;    // [004000..007fff] 1 register (8-Bytes) per hart
    CLINT_MTIME_TYPE mtime;          // [00bff8] 1 register for all hart

    uint64_t update_time_;          // Last time when mtime was updated
};

DECLARE_CLASS(CLINT)

}  // namespace debugger
