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
    static const uint32_t IR_DTMCS = 0x10;
    static const uint32_t IR_DMI = 0x11;
    static const uint32_t IR_BYPASS = 0x1F;

    // DTM Control and Status
    union DtmcsType {
        uint32_t u32;
        struct bits_type {
            uint32_t version : 4;   // [3:0] R:
            uint32_t abits : 6;     // [9:4] R:
            uint32_t dmistat : 2;   // [11:10] R:
            uint32_t idle : 3;      // [14:12] R: 0=it is not necessary to enter Run-Test/Idle at all;
            uint32_t rsrv15 : 1;    // [15]
            uint32_t dmireset : 1;          // [16] W1: Clears sticky error state, but does not affect DMI transaction
            uint32_t dmihardreset : 1;      // [17] W1: Only 1 takes effect
            uint32_t rsrv31_18 : 14;        // [31:18]
        } bits;
    };

    // Debug Module Interface Access
    union DmiType {
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

    // Debug Module Debug Bus Registers:
    static const uint32_t DMI_ABSTRACT_DATA0 = 0x04;
    static const uint32_t DMI_ABSTRACT_DATA1 = 0x05;
    static const uint32_t DMI_ABSTRACT_DATA2 = 0x06;
    static const uint32_t DMI_ABSTRACT_DATA3 = 0x07;
    static const uint32_t DMI_ABSTRACT_DATA4 = 0x08;
    static const uint32_t DMI_ABSTRACT_DATA5 = 0x09;
    static const uint32_t DMI_ABSTRACT_DATA6 = 0x0A;
    static const uint32_t DMI_ABSTRACT_DATA7 = 0x0B;
    static const uint32_t DMI_ABSTRACT_DATA8 = 0x0C;
    static const uint32_t DMI_ABSTRACT_DATA9 = 0x0D;
    static const uint32_t DMI_ABSTRACT_DATA10 = 0x0E;
    static const uint32_t DMI_ABSTRACT_DATA11 = 0x0F;
    static const uint32_t DMI_DMCONTROL = 0x10;
    static const uint32_t DMI_DMSTATUS = 0x11;
    static const uint32_t DMI_HARTINFO = 0x12;
    static const uint32_t DMI_HALTSUM1 = 0x13;              // Halt summary 1
    static const uint32_t DMI_HAWINDOWSEL = 0x14;
    static const uint32_t DMI_HAWINDOW = 0x15;
    static const uint32_t DMI_ABSTRACTCS = 0x16;
    static const uint32_t DMI_COMMAND = 0x17;
    static const uint32_t DMI_ABSTRACTAUTO = 0x18;
    static const uint32_t DMI_CONFSTRPTR0 = 0x19;
    static const uint32_t DMI_CONFSTRPTR1 = 0x1A;
    static const uint32_t DMI_CONFSTRPTR2 = 0x1B;
    static const uint32_t DMI_CONFSTRPTR3 = 0x1C;
    static const uint32_t DMI_NEXTDM = 0x1D;                // Next Debug Module
    static const uint32_t DMI_CUSTOM = 0x1f;
    static const uint32_t DMI_PROGBUF0 = 0x20;
    static const uint32_t DMI_PROGBUF1 = 0x21;
    static const uint32_t DMI_PROGBUF2 = 0x22;
    static const uint32_t DMI_PROGBUF3 = 0x23;
    static const uint32_t DMI_PROGBUF4 = 0x24;
    static const uint32_t DMI_PROGBUF5 = 0x25;
    static const uint32_t DMI_PROGBUF6 = 0x26;
    static const uint32_t DMI_PROGBUF7 = 0x27;
    static const uint32_t DMI_PROGBUF8 = 0x28;
    static const uint32_t DMI_PROGBUF9 = 0x29;
    static const uint32_t DMI_PROGBUF10 = 0x2A;
    static const uint32_t DMI_PROGBUF11 = 0x2B;
    static const uint32_t DMI_PROGBUF12 = 0x2C;
    static const uint32_t DMI_PROGBUF13 = 0x2D;
    static const uint32_t DMI_PROGBUF14 = 0x2E;
    static const uint32_t DMI_PROGBUF15 = 0x2F;
    static const uint32_t DMI_AUTHDATA = 0x30;
    static const uint32_t DMI_DMCS2 = 0x32;
    static const uint32_t DMI_HALTSUM2 = 0x34;              // Halt summary 2
    static const uint32_t DMI_HALTSUM3 = 0x35;              // Halt summary 3
    static const uint32_t DMI_SBADDRESS3 = 0x37;            // System Bus Address 127:96
    static const uint32_t DMI_SBCS = 0x38;                  // System Bus Address Control and Status
    static const uint32_t DMI_SBADDRESS0 = 0x39;            // System Bus Address 31:0
    static const uint32_t DMI_SBADDRESS1 = 0x3A;            // System Bus Address 63:32
    static const uint32_t DMI_SBADDRESS2 = 0x3B;            // System Bus Address 95:64
    static const uint32_t DMI_SBDATA0 = 0x3C;               // System Bus Data 31:0
    static const uint32_t DMI_SBDATA1 = 0x3D;               // System Bus Data 63:32
    static const uint32_t DMI_SBDATA2 = 0x3E;               // System Bus Data 95:64
    static const uint32_t DMI_SBDATA3 = 0x3F;               // System Bus Data 127:96
    static const uint32_t DMI_HALTSUM0 = 0x40;              // Halt summary 0
    // The following requires ABITS >= 8
    static const uint32_t DMI_CUSTOM0 = 0x70;               // Custom Feature 0
    static const uint32_t DMI_CUSTOM1 = 0x71;               // Custom Feature 1
    static const uint32_t DMI_CUSTOM2 = 0x72;               // Custom Feature 2
    static const uint32_t DMI_CUSTOM3 = 0x73;               // Custom Feature 3
    static const uint32_t DMI_CUSTOM4 = 0x74;               // Custom Feature 4
    static const uint32_t DMI_CUSTOM5 = 0x75;               // Custom Feature 5
    static const uint32_t DMI_CUSTOM6 = 0x76;               // Custom Feature 6
    static const uint32_t DMI_CUSTOM7 = 0x77;               // Custom Feature 7
    static const uint32_t DMI_CUSTOM8 = 0x78;               // Custom Feature 8
    static const uint32_t DMI_CUSTOM9 = 0x79;               // Custom Feature 9
    static const uint32_t DMI_CUSTOM10 = 0x7A;              // Custom Feature 10
    static const uint32_t DMI_CUSTOM11 = 0x7B;              // Custom Feature 11
    static const uint32_t DMI_CUSTOM12 = 0x7C;              // Custom Feature 12
    static const uint32_t DMI_CUSTOM13 = 0x7D;              // Custom Feature 13
    static const uint32_t DMI_CUSTOM14 = 0x7E;              // Custom Feature 14
    static const uint32_t DMI_CUSTOM15 = 0x7F;              // Custom Feature 15
    
    // Debug module Status (dmstatus, at 0x11)
    union dmi_dmstatus_type {
        uint32_t u32;
        struct bits_type {
            uint32_t version : 4;           // [3:0] R. 0=none; 1=0.11; 2=0.13; 3=1.0; 15=custom
            uint32_t confstrptrvalid : 1;   // [4]
            uint32_t hasresethalreq : 1;    // [5] R. 1 if DM supports Halt-on-reset
            uint32_t authbusy : 1;          // [6]
            uint32_t authenticated : 1;     // [7]
            uint32_t anyhalted : 1;         // [8]
            uint32_t allhalted : 1;         // [9]
            uint32_t anyrunning : 1;        // [10]
            uint32_t allrunning : 1;        // [11]
            uint32_t anyunavail : 1;        // [12]
            uint32_t allunavail : 1;        // [13]
            uint32_t anynonexistent : 1;    // [14]
            uint32_t allnonexistent : 1;    // [15]
            uint32_t anyresumeack : 1;      // [16]
            uint32_t allresumeack : 1;      // [17]
            uint32_t anyhavereset : 1;      // [18]
            uint32_t allhavereset : 1;      // [19]
            uint32_t rsrv21_20 : 2;         // [21:20]
            uint32_t impebreak : 1;         // [22]
            uint32_t stickyunavail : 1;     // [23]
            uint32_t ndmresetpending : 1;   // [24]
            uint32_t rsrv31_25 : 7;         // [31:25]
        } bits;
    };

    // Dub Module Control (dmcontrol, at 0x10)
    union dmi_dmcontrol_type {
        uint32_t u32;
        struct bits_type {
            uint32_t dmactive : 1;          // [0] RW. 1(active)=Function normally. 0(inactive). Only dmactive can be written.
            uint32_t ndmreset : 1;          // [1] RW. Reset all hardware except DM
            uint32_t clrresethaltreq : 1;   // [2] W1.Clear Halt-on-reset
            uint32_t setresethaltreq : 1;   // [3] W1. Set Halt-on-reset for sleceted harts
            uint32_t clrkeepalive : 1;      // [4] W1. Clears keepalive. Writes apply to the new of hartsel and hasel
            uint32_t setkeepalive : 1;      // [5] W1. Set keepalive for all selected harts unless clrkeepalive set to 1. Apply to the new hartsel.
            uint32_t hartselhi : 10;        // [15:6] WARL.
            uint32_t hartsello : 10;        // [25:16] WARL.
            uint32_t hasel : 1;             // [26] WARL. Definition of currently selected hartss: 0(single); 1(multiple)
            uint32_t ackunavail : 1;        // [27] W1. 0(nop)=No effect; 1(ack)=clears unavail for any selected harts
            uint32_t ackhavereset : 1;      // [28] WARL. 0(nop)=No effect; 1(ack)=Clears havereset for any selected harts.
            uint32_t hartreset : 1;         // [29] WARL. Reset select harts. Debugger writes 1 and then 0
            uint32_t resumereq : 1;         // [30] W1. Resume once if the halted. It clears resume ack bit. Ignored if haltreq=1
            uint32_t haltreq : 1;           // [31] WARZ. 1 set halt request
        } bits;
    };

    // Abstract Control and Status (abstractcs, at 0x16)
    union dmi_abstractcs_type {
        uint32_t u32;
        struct bits_type {
            uint32_t datacount : 4;         // [3:0] R. Number of implemented data registers
            uint32_t rsrv7_4 : 4;           // [7:4]
            uint32_t cmderr : 3;            // [10:8] R/W1C. Write 1 clears bits. 0=No errors; 1=Busy; 2=not supported; 3=exception; 4=halt/resume; 5=bus; 6=reserved; 7=other
            uint32_t relaxedpriv : 1;       // [11] WARL. 0=Full permission check; 1=Relaxed permission checks applied
            uint32_t busy : 1;              // [12] R. 0=Ready; 1=Busy
            uint32_t rsrv23_13 : 11;        // [23:13]
            uint32_t progbufsize : 5;       // [28:24] R. Size of Program Buffer. Valid size are 0-16
            uint32_t rsrv31_29 : 3;
        } bits;
    };

    static const uint32_t DMI_ABSTRACTCS_CMDERR_NONE = 0;
    static const uint32_t DMI_ABSTRACTCS_CMDERR_BUSY = 1;
    static const uint32_t DMI_ABSTRACTCS_CMDERR_NOT_SUPPORTED = 2;
    static const uint32_t DMI_ABSTRACTCS_CMDERR_EXCEPTION = 3;
    static const uint32_t DMI_ABSTRACTCS_CMDERR_HALTRESUME = 4;
    static const uint32_t DMI_ABSTRACTCS_CMDERR_BUS = 5;

    // Abstract command (command, at 0x17):
    union dmi_command_type {
        uint32_t u32;
        struct regaccess_type {
            uint32_t regno : 16;                // [15:0]
            uint32_t write : 1;                 // [16]
            uint32_t transfer : 1;              // [17]
            uint32_t postexec : 1;              // [18]
            uint32_t aarpostincrement : 1;      // [19]
            uint32_t aarsize : 3;               // [22:20]
            uint32_t rsrv23 : 1;                // [23]
            uint32_t cmdtype : 8;               // [31:24], type 0
        } regaccess;

        struct quickaccess_type {
            uint32_t zero23_0 : 24;             // [23:0] zeros
            uint32_t cmdtype : 8;               // [31:24], type 1
        } quickaccess;

        struct memaccess_type {
            uint32_t rsrv13_0 : 14;             // [13:0]
            uint32_t target_specific : 2;       // [15:14]
            uint32_t write : 1;                 // [16]
            uint32_t rsrv18_17 : 2;             // [18:17]
            uint32_t aampostincrement : 1;      // [19]
            uint32_t aamsize : 3;               // [22:20]
            uint32_t aamvirtual : 1;            // [23]
            uint32_t cmdtype : 8;               // [31:24], type = 2
        } memaccess;
    };

    static const uint32_t CMD_AAxSIZE_32BITS = 0x2;
    static const uint32_t CMD_AAxSIZE_64BITS = 0x3;
    static const uint32_t CMD_AAxSIZE_128BITS = 0x4;

    virtual const char *getBrief() { return IJtag_brief; }

    virtual const char *getDetail() { return IJtag_detail; }

    virtual void resetAsync() = 0;
    virtual void resetSync() = 0;
    virtual uint32_t scanIdCode() = 0;
    virtual DtmcsType scanDtmcs() = 0;
    virtual uint32_t scanDmi(uint32_t addr, uint32_t data, EDmiOperation op) = 0;
};

}  // namespace debugger

