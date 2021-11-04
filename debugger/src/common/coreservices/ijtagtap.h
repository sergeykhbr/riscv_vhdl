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

static const char *const IFACE_JTAG_TAP = "IJtagTap";

static const char *const IJtagTap_brief =
"Test Access Point (TAP) pin compatible interface.";

static const char *const IJtagTap_detail =
"This interface is used for the bitbang access to the simulated "
"hardware from the OpenOCD utility.";

class IJtagTap : public IFace {
 public:
    IJtagTap() : IFace(IFACE_JTAG_TAP) {}

    virtual const char *getBrief() { return IJtagTap_brief; }

    virtual const char *getDetail() { return IJtagTap_detail; }

    virtual void resetTAP(char trst, char srst) = 0;
    virtual void setPins(char tck, char tms, char tdi) = 0;
    virtual bool getTDO() = 0;
};

}  // namespace debugger

