/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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
#include <attribute.h>

namespace debugger {

static const char *const IFACE_JTAG_BITBANG = "IJtagBitBang";

static const char *const IJtagBitBang_brief =
"JTAG Bit Bang interface.";

static const char *const IJtagBitBang_detail =
"This interface is used for the bitbang access to the simulated "
"hardware from the OpenOCD utility.";

class IJtagBitBang : public IFace {
 public:
    IJtagBitBang() : IFace(IFACE_JTAG_BITBANG) {}

    virtual const char *getBrief() { return IJtagBitBang_brief; }

    virtual const char *getDetail() { return IJtagBitBang_detail; }

    virtual void resetTAP(char trst, char srst) = 0;
    virtual void setPins(char tck, char tms, char tdi) = 0;
    virtual bool getTDO() = 0;
};

}  // namespace debugger

