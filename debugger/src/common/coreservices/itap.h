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
 *
 * @brief      General interface of the hardware access.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_ITAP_H__
#define __DEBUGGER_COMMON_CORESERVICES_ITAP_H__

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>

namespace debugger {

static const char *const IFACE_TAP = "ITap";

static const char *const ITap_brief =
"Test Access Point (TAP) software interface.";

static const char *const ITap_detail =
"This interface is used for the direct access to the Hardware. "
"Typically, it is doing via JTAG or other transport interface.";

static const int TAP_ERROR = -1;

class ITap : public IFace {
 public:
    ITap() : IFace(IFACE_TAP) {}

    virtual const char *getBrief() { return ITap_brief; }

    virtual const char *getDetail() { return ITap_detail; }

    virtual int read(uint64_t addr, int bytes, uint8_t *obuf) = 0;
    virtual int write(uint64_t addr, int bytes, uint8_t *ibuf) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ITAP_H__
