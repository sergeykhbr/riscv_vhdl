/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_COMMON_CORESERVICES_ICPUGEN_H__
#define __DEBUGGER_COMMON_CORESERVICES_ICPUGEN_H__

#include <inttypes.h>
#include <iface.h>
#include "coreservices/imemop.h"

namespace debugger {

static const char *const IFACE_CPU_GENERIC = "ICpuGeneric";
static const uint64_t REG_INVALID   = ~0ull;

struct DebugPortTransactionType {
    bool write;
    uint8_t region;
    uint16_t addr;
    uint32_t bytes;
    uint64_t wdata;
    uint64_t rdata;
};

static const char *const IFACE_DBG_NB_RESPONSE = "IDbgNbResponse";

class IDbgNbResponse : public IFace {
 public:
    IDbgNbResponse() : IFace(IFACE_DBG_NB_RESPONSE) {}

    virtual void nb_response_debug_port(DebugPortTransactionType *trans) = 0;
};

class ICpuGeneric : public IFace {
 public:
    ICpuGeneric() : IFace(IFACE_CPU_GENERIC) {}

    virtual bool isHalt() = 0;
    virtual void raiseSignal(int idx) = 0;
    virtual void lowerSignal(int idx) = 0;
    virtual void nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICPUGEN_H__
