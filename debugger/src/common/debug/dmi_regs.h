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
#include "coreservices/idmi.h"
#include "generic/mapreg.h"

namespace debugger {

class DebugRegisterType : public MappedReg32Type {
 public:
    DebugRegisterType(IService* parent, const char* name, uint64_t addr) :
        MappedReg32Type(parent, name, addr), idmi_(0) {}

 protected:
    IDmi* getpIDmi();

 protected:
    IDmi* idmi_;
};

class DmiRegisterBankType : public GenericReg32Bank {
 public:
    DmiRegisterBankType(IService* parent, const char* name, uint64_t addr, int len) :
        GenericReg32Bank(parent, name, addr, len), idmi_(0) {}

 protected:
    IDmi* getpIDmi();

 protected:
    IDmi* idmi_;
};

class DATABUF_TYPE : public DmiRegisterBankType {
 public:
    DATABUF_TYPE(IService *parent, const char *name, uint64_t addr) :
        DmiRegisterBankType(parent, name, addr, 6) {}

 protected:
    virtual uint32_t read(int idx) override;
    virtual void write(int idx, uint32_t val) override;
};

class PROGBUF_TYPE : public DmiRegisterBankType {
 public:
    PROGBUF_TYPE(IService *parent, const char *name, uint64_t addr) :
        DmiRegisterBankType(parent, name, addr, 16) {}

    virtual uint32_t read(int idx) override;
    virtual void write(int idx, uint32_t val) override;
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
            uint32_t clearresethaltreq : 1; // [2] clear halt on reset
            uint32_t setresethaltreq : 1;   // [3] set halt on reset
            uint32_t rsrv5_4 : 2;           // [5:4]
            uint32_t hartselhi : 10;        // [15:6]
            uint32_t hartsello : 10;        // [25:16]
            uint32_t hasel : 1;             // [26]
            uint32_t rsrv27  : 1;           // [27]
            uint32_t ackhavereset : 1;      // [28]
            uint32_t hartreset : 1;         // [29]
            uint32_t resumereq : 1;         // [30]
            uint32_t haltreq : 1;           // [31]
        } bits;
    };

    uint32_t hartsel() {
        ValueType v;
        v.val = getValue().val;
        return (v.bits.hartselhi << 10) | v.bits.hartsello;
    }

 protected:
    virtual uint32_t aboutToWrite(uint32_t new_val) override;
};

class HARTINFO_TYPE : public DebugRegisterType {
 public:
    HARTINFO_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[4];
        struct {
            uint32_t dataaddr : 12;         // [11:0] 
            uint32_t datasize : 4;          // [15:12]
            uint32_t dataaccess : 1;        // [16] 0=data regs shadows in CSR; 1=data regs shadowed in memory
            uint32_t rsv19_17 : 3;          // [19:17]
            uint32_t nscratch : 4;          // [23:20]  Number of dscratch registers available for the debugger 
            uint32_t rsv31_24 : 8;          // [31:24]
        } bits;
    };

 protected:
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
            uint32_t rsrv4 : 1;             // [4]
            uint32_t hasresethaltreq : 1;   // [5]  Halt on reset capability
            uint32_t rsrv6 : 1;             // [6]
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

class ABSTRACTCS_TYPE : public DebugRegisterType {
 public:
    ABSTRACTCS_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[4];
        struct {
            uint32_t datacount : 4;         // [3:0]
            uint32_t rsrv7_4 : 4;           // [7:4]
            uint32_t cmderr : 3;            // [10:8]
            uint32_t rsrv11 : 1;            // [11]
            uint32_t busy : 1;              // [12]
            uint32_t rsrv23_13 : 11;        // [23:13]
            uint32_t progbufsize : 5;       // [28:24]
            uint32_t rsrv31_29 : 3;         // [31:29]
        } bits;
    };

    void set_cmderr(uint32_t val) {
        ValueType t;
        t.val = getValue().val;
        t.bits.cmderr = val;
        setValue(t.val);
    }
    uint32_t get_cmderr() {
        ValueType t;
        t.val = getValue().val;
        return t.bits.cmderr;
    }

 protected:
    virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
    virtual uint32_t aboutToRead(uint32_t cur_val) override;
};

class ABSTRACTAUTO_TYPE : public DebugRegisterType {
 public:
    ABSTRACTAUTO_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[4];
        struct {
            uint32_t autoexecdata : 12;     // [11:0]
            uint32_t rsrv15_12 : 4;         // [15:12]
            uint32_t autoexecprogbuf : 16;  // [31:16]
        } bits;
    };

    virtual bool isAutoexecData(int idx) {
        ValueType t;
        t.val = getValue().val;
        return (t.bits.autoexecdata >> idx) & 0x1;
    }
    virtual bool isAutoexecProgbuf(int idx) {
        ValueType t;
        t.val = getValue().val;
        return (t.bits.autoexecprogbuf >> idx) & 0x1;
    }
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

    void execute();

    virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
};

class SBCS_TYPE : public DebugRegisterType {
 public:
    SBCS_TYPE(IService *parent, const char *name, uint64_t addr) :
        DebugRegisterType(parent, name, addr) {}

    union ValueType {
        uint32_t val;
        uint8_t u8[4];
        struct {
            uint32_t sbaccess8 : 1;         // [0] 
            uint32_t sbaccess16 : 1;        // [1] 
            uint32_t sbaccess32 : 1;        // [2] 
            uint32_t sbaccess64 : 1;        // [3] 
            uint32_t sbaccess128 : 1;       // [4] 
            uint32_t sbasize : 7;           // [11:5]
            uint32_t sberror : 3;           // [14:12]
            uint32_t sbreadondata : 1;      // [15]
            uint32_t sbautoincrement : 1;   // [16]
            uint32_t sbaccess : 3;          // [19:17]
            uint32_t sbreadonaddr : 1;      // [20]
            uint32_t sbbusy : 1;            // [21]
            uint32_t sbbusyerror : 1;       // [22]
            uint32_t rsv28_23 : 6;          // [28:23]
            uint32_t sbversion : 3;         // [31:29]
        } bits;
    };

 protected:
    virtual uint32_t aboutToRead(uint32_t cur_val) override;
    virtual uint32_t aboutToWrite(uint32_t nxt_val) override;
};

}  // namespace debugger

