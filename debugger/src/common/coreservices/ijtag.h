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

    enum ETapState {
        RESET,
        IDLE,
        DRSCAN,
        DRCAPTURE,
        DRSHIFT,
        DREXIT1,
        DRPAUSE,
        DREXIT2,
        DRUPDATE,
        IRSCAN,
        IRCAPTURE,
        IRSHIFT,
        IREXIT1,
        IRPAUSE,
        IREXIT2,
        IRUPDATE
    };

    static const uint32_t IR_IDCODE = 0x01;
    static const uint32_t IR_DTMCONTROL = 0x10;
    static const uint32_t IR_DBUS = 0x11;
    static const uint32_t IR_BYPASS = 0x1F;

    union DtmControlType {
        uint32_t u32;
        struct bits_type {
            uint32_t version : 4;   // [3:0]
            uint32_t abits : 6;     // [9:4]
            uint32_t stat : 2;      // [11:10]
            uint32_t rsrv : 20;
        } bits;
    };

    union DmiRegisterType {
        uint64_t u64;
        struct bits_type {
            uint64_t status : 2;    // [1:0] status on read. On write [0]=rena, [1]=wena
            uint64_t data : 32;     // [33:2]
            uint64_t addr : 7;      // [34+abits-1:34]
        } bits;
    };

    enum EDmiOperation {
        DmiOp_None,
        DmiOp_Read,
        DmiOp_Write,
        DmiOp_ReadWrite,
    };

    virtual const char *getBrief() { return IJtag_brief; }

    virtual const char *getDetail() { return IJtag_detail; }

    virtual void resetAsync() = 0;
    virtual void resetSync() = 0;
    virtual uint32_t scanIdCode() = 0;
    virtual DtmControlType scanDtmControl() = 0;
    virtual uint32_t scanDmiBus(uint32_t addr, uint32_t data, EDmiOperation op) = 0;
};

}  // namespace debugger

