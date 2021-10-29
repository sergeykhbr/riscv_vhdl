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
#include <iservice.h>
#include "coreservices/ireset.h"
#include "coreservices/idmi.h"
#include "coreservices/idport.h"
#include "generic/rmembank_gen1.h"
#include "debug/dmi_regs.h"

namespace debugger {

class DmiFunctional : public RegMemBankGeneric,
                      public IDmi {
 public:

    explicit DmiFunctional(const char *name);
    virtual ~DmiFunctional();

    /** IService interface */
    virtual void postinitService() override;

    /** IDmi interface */
    // Must be 2*n value
    virtual int getCpuMax() { return cpumax_.to_uint32(); }
    virtual int getRegTotal() { return dataregTotal_.to_uint32(); }
    virtual int getProgbufTotal() { return progbufTotal_.to_uint32(); }
    virtual uint32_t getHartSelected() {
        return dmcontrol.hartsel();
    }
    // Returns resumeack on success
    virtual void set_resumereq(uint32_t hartsel) {
        if (phartdata_[hartsel].idport) {
            phartdata_[hartsel].idport->resume();
            phartdata_[hartsel].resumeack = 1;
        }
    }
    virtual bool get_resumeack(uint32_t hartsel) {
        return phartdata_[hartsel].resumeack;
    }
    virtual void set_haltreq(uint32_t hartsel) {
        if (phartdata_[hartsel].idport) {
            phartdata_[hartsel].idport->halt();
        }
    }
    virtual void set_hartreset(uint32_t hartsel) {
    }
    virtual void clear_hartreset(uint32_t hartsel) {
    }
    // Halt on reset request
    virtual void set_resethaltreq(uint32_t hartsel) {
    }
    virtual void clear_resethaltreq(uint32_t hartsel) {
    }
    // Full system reset
    virtual void set_ndmreset() {
    }
    virtual void clear_ndmreset() {
    }

    virtual void set_cmderr(ECmdErrType cmderr) {
        abstractcs.set_cmderr(cmderr);
    }
    virtual ECmdErrType get_cmderr() {
        return static_cast<ECmdErrType>(abstractcs.get_cmderr());
    }

    virtual bool isHalted(uint32_t hartsel) {
        if (phartdata_[hartsel].idport) {
            return phartdata_[hartsel].idport->isHalted();
        }
        return false;
    }
    virtual bool isAvailable(uint32_t hartsel) {
        if (phartdata_[hartsel].idport) {
            return true;
        }
        return false;
    }

    virtual void readTransfer(uint32_t regno, uint32_t size);
    virtual void writeTransfer(uint32_t regno, uint32_t size);

    virtual bool isDataAutoexec(int idx) {
        return false;
    }

    virtual void executeCommand() {}


 private:
    AttributeType sysbus_;
    AttributeType cpumax_;
    AttributeType dataregTotal_;
    AttributeType progbufTotal_;
    AttributeType hartlist_;

    IMemoryOperation *ibus_;

    struct HartDataType {
        IDPort *idport; // if 0, hart is not available
        bool resumeack;
    } *phartdata_;

    static const unsigned DM_STATE_IDLE = 0;
    static const unsigned DM_STATE_ACCESS = 1;

    static const unsigned CMD_STATE_IDLE    = 0;
    static const unsigned CMD_STATE_INIT    = 1;
    static const unsigned CMD_STATE_REQUEST = 2;
    static const unsigned CMD_STATE_RESPONSE = 3;
    static const unsigned CMD_STATE_WAIT_HALTED = 4;

    union g {
        struct DmiRegsType {
            uint32_t rsrv_00_03[4];     //
            uint32_t data[12];          // 0x04 Abstract Data
            uint32_t dmcontrol;         // 0x10 Debug Module Control
            uint32_t dmstatus;          // 0x11 Debug Module Status
            uint32_t hartinfo;          // 0x12 Hart Info
            uint32_t haltsum1;          // 0x13 Halt Summary 1
            uint32_t hawindowsel;       // 0x14 Hart Array Window Select
            uint32_t hawindow;          // 0x15 Hart Array Window
            uint32_t abstractcs;        // 0x16 Abstract Control and Status
            uint32_t command;           // 0x17 Abstract Command
            uint32_t abstractauto;      // 0x18 Abstract Command Autoexec
            uint32_t confstrptr[4];     // 0x19-0x1c Configuration String Pointers 1-3
            uint32_t nextdm;            // 0x1d Next Debug Module
            uint32_t rsrv_1e_1f[2];
            uint32_t progbuf[16];       // 0x20-0x2f Program buffer 0-15
            uint32_t authdata;          // 0x30 Authentication Data
            uint32_t rsrv_31_33[3];
            uint32_t haltsum2;          // 0x34 Halt summary 2
            uint32_t haltsum3;          // 0x35 Halt summary 3
            uint32_t rsrv_36;
            uint32_t sbaddress3;        // 0x37 System Bus Address [127:96]
            uint32_t sbcs;              // 0x38 System Bus Access Control and Status
            uint32_t sbaddress0;        // 0x39 System Bus Address [31:0]
            uint32_t sbaddress1;        // 0x3a System Bus Address [63:32]
            uint32_t sbaddress2;        // 0x3b System Bus Address [95:64]
            uint32_t sbdata0;           // 0x3a System Bus Data [31:0]
            uint32_t sbdata1;           // 0x3a System Bus Data [63:32]
            uint32_t sbdata2;           // 0x3a System Bus Data [95:64]
            uint32_t sbdata3;           // 0x3a System Bus Data [127:96]
            uint32_t haltsum0;          // 0x40 Halt Summary 0
        } r;
        uint32_t b8[sizeof(DmiRegsType)];
        uint32_t b32[sizeof(DmiRegsType) / sizeof(uint32_t)];
    };

    DMDATAx_TYPE data0;
    DMDATAx_TYPE data1;
    DMDATAx_TYPE data2;
    DMDATAx_TYPE data3;
    DMCONTROL_TYPE dmcontrol;   // 0x10
    DMSTATUS_TYPE dmstatus;     // 0x11;
    ABSTRACTCS_TYPE abstractcs; // 0x16
    COMMAND_TYPE command;       // 0x17
    HALTSUM0_TYPE haltsum0;     // 0x40
};

DECLARE_CLASS(DmiFunctional)

}  // namespace debugger

