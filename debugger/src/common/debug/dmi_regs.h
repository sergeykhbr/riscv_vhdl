/**
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <iservice.h>
#include "coreservices/idebug.h"
#include "generic/mapreg.h"

namespace debugger {

class DebugRegisterType : public MappedReg32Type {
 public:
    DebugRegisterType(IService* parent, const char* name, uint64_t addr) :
        MappedReg32Type(parent, name, addr), idbg_(0) {}

 protected:
    IDebug* getpIDebug();

 protected:
    IDebug* idbg_;
};

class DMDATAx_TYPE : public DebugRegisterType {
 public:
    DMDATAx_TYPE(IService *parent, const char *name, int idx, uint64_t addr) :
        DebugRegisterType(parent, name, addr), idx_(idx) {}

 protected:
    virtual uint32_t aboutToWrite(uint32_t new_val) override;
 private:
    int idx_;
};

class DMCONTROL_TYPE : public DebugRegisterType {
 public:
    DMCONTROL_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[4];
        struct {
            uint32_t dmactive : 1;          // [0] 1=module functioning normally
            uint32_t ndmreset : 1;          // [1] 1=system reset
            uint32_t rsrv5_2 : 4;           // [5:2]
            uint32_t hartselhi : 10;        // [15:6]
            uint32_t hartsello : 10;        // [25:16]
            uint32_t rsrv29_26  : 4;        // [29:26]
            uint32_t resumereq : 1;         // [30]
            uint32_t haltreq : 1;           // [31]
        } bits;
    };

 protected:
    virtual uint32_t aboutToWrite(uint32_t new_val) override;
    virtual uint32_t aboutToRead(uint32_t cur_val) override;
};

class DMSTATUS_TYPE : public DebugRegisterType {
 public:
    DMSTATUS_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[4];
        struct {
            uint32_t version : 4;           // [3:0] 2=version 0.13
            uint32_t rsrv6_4 : 3;           // [6:4]
            uint32_t authenticated : 1;     // [7]
            uint32_t anyhalted : 1;         // [8]
            uint32_t allhalted : 1;         // [9]
            uint32_t anyrunning : 1;        // [10]
            uint32_t allrunning : 1;        // [11]
            uint32_t anyunavail : 1;        // [12]
            uint32_t allunavail : 1;        // [13]
            uint32_t anynonexistent: 1;     // [14]
            uint32_t allnonexistent: 1;     // [15]
            uint32_t anyresumeack: 1;       // [16]
            uint32_t allresumeack: 1;       // [17]
            uint32_t rsrv21_18  : 4;        // [21:18]
            uint32_t impebreak : 1;         // [22]
            uint32_t rsv      : 9;          // [31:23]
        } bits;
    };

 protected:
    virtual uint32_t aboutToRead(uint32_t cur_val) override;
};

class HALTSUM0_TYPE : public DebugRegisterType {
 public:
    HALTSUM0_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}
 protected:
    virtual uint32_t aboutToRead(uint32_t cur_val) override;
};


class DCSR_TYPE : public DebugRegisterType {
 public:
    DCSR_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[4];
        struct bits_type {
            uint32_t prv : 2;       // [1:0] 
            uint32_t step : 1;      // [2]
            uint32_t rsv5_3 : 3;    // [5:3]
            uint32_t cause : 3;     // [8:6]
            uint32_t stoptime : 1;  // [9]
            uint32_t stopcount : 1; // [10]
            uint32_t rsv11 : 1;     // [11]
            uint32_t ebreaku : 1;   // [12]
            uint32_t ebreaks : 1;   // [13]
            uint32_t ebreakh : 1;   // [14]
            uint32_t ebreakm : 1;   // [15]
            uint32_t rsv27_16 : 12; // [27:16]
            uint32_t xdebugver : 4; // [31:28] 0=no external debug support; 4=exists as in spec 0.13
        } bits;
    };
};

class COMMAND_TYPE : public DebugRegisterType {
 public:
    COMMAND_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[8];
        struct bits_type {
            uint32_t regno : 16;    // [15:0] 0=copy data from regiter into arg0; 1=copy arg0 to register
            uint32_t write : 1;     // [16]
            uint32_t transfer : 1;  // [17] 0=don;t do operation specified by write; 1=do operation specified by write
            uint32_t postexec : 1;  // [18] 0=no effect; 1=execute prog from buffer after trnsfer
            uint32_t aarpostincrement : 1;   // [19] 0=no effect; 1=increment regno
            uint32_t aarsize : 3;   // [22:20] 2=32 bits; 3=64 bits; 4=128 bits access
            uint32_t rsv23 : 1;     // [23]
            uint32_t cmdtype : 8;   // [31:24] 0=Access regsiter
        } bits;
    };

    virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
};

}  // namespace debugger

