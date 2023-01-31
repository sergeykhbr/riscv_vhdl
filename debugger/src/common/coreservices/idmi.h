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

static const char *const IFACE_DMI = "IDmi";

enum ECmdErrType {
    CMDERR_NONE = 0,
    CMDERR_BUSY = 1,
    CMDERR_NOTSUPPROTED = 2,
    CMDERR_EXCEPTION = 3,
    CMDERR_WRONGSTATE = 4,
    CMDERR_BUSERROR = 5,
    CMDERR_OTHERS = 7,
};

enum EDmistatus {
    DMI_STAT_SUCCESS = 0,
    DMI_STAT_RESERVED = 1,
    DMI_STAT_FAILED = 2,
    DMI_STAT_BUSY = 3
};


class IDmi : public IFace {
 public:
    IDmi() : IFace(IFACE_DMI) {}

    virtual void dtm_dmireset() = 0;
    virtual void dtm_dmihardreset() = 0;
    virtual uint32_t dmi_read(uint32_t addr, uint32_t *rdata) = 0;
    virtual uint32_t dmi_write(uint32_t addr, uint32_t wdata) = 0;
    virtual EDmistatus dmi_status() = 0;
};

}  // namespace debugger

