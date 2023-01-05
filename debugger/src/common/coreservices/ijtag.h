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
 *
 * @brief      General interface of the hardware access.
 */

#pragma once

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>

namespace debugger {

static const char *const IFACE_JTAG = "IJtag";

static const char *const IJtag_brief =
"JTAG bus interface.";

static const char *const IJtag_detail =
"This interface is used for the emulating AR/DR scan requests.";

class IJtag : public IFace {
 public:
    IJtag() : IFace(IFACE_JTAG) {}

    virtual const char *getBrief() { return IJtag_brief; }

    virtual const char *getDetail() { return IJtag_detail; }

    virtual void TestReset() = 0;
    virtual uint64_t IR(uint64_t iscan, int sz) = 0;
    virtual uint64_t DR(uint64_t dscan, int sz) = 0;
};

}  // namespace debugger

