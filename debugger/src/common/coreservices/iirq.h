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

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_IRQ_CONTROLLER = "IIrqController";

static const int IRQ_REQUEST_NONE = 0;

// Context id can any value, for PLIC for an example:
// Example 1: (for PLIC)
//   HART0_M = 0
//   HART0_S = 1
//   HART1_M = 2
//   etc
// Example 2: (for CLINT)
//   HART0_SOFTWARE_IRQ = 0
//   HART0_TIMER_IRQ = 1
//   HART1_SOFTWARE_IRQ = 2
//   HART1_TIMER_IRQ = 3
//   etc

class IIrqController : public IFace {
 public:
    IIrqController() : IFace(IFACE_IRQ_CONTROLLER) {}

    // Request from periphery to CPU
    virtual int requestInterrupt(IFace *isrc, int idx) = 0;

    // Interrupt should be requested, enabled with proper 
    // prioiry and enabled for context. Called by CPU.
    // @ret IRQ_REQUEST_NONE if no requests
    virtual int getPendingRequest(int ctxid) = 0;
};

}  // namespace debugger
