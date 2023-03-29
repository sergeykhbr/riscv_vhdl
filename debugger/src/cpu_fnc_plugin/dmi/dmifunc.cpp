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
 */

#include "dmifunc.h"
#include <riscv-isa.h>

namespace debugger {

DmiFunctional::DmiFunctional(const char *name)
    : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<IDmi *>(this));

    registerAttribute("SysBus", &sysbus_);
    registerAttribute("SysBusMasterID", &busid_);
    registerAttribute("CpuMax", &cpumax_);
    registerAttribute("DataregTotal", &dataregTotal_);
    registerAttribute("ProgbufTotal", &progbufTotal_);
    registerAttribute("HartList", &hartlist_);

    phartdata_ = 0;
    ndmreset_ = 0;
    hartsel_ = 0;
    arg0_.val = 0;
    arg1_.val = 0;
    arg2_.val = 0;
    autoexecdata_ = 0;
    autoexecprogbuf_ = 0;
    command_.u32 = 0;
    cmderr_ = 0;
    memset(progbuf_, 0, sizeof(progbuf_));
}

DmiFunctional::~DmiFunctional() {
    if (phartdata_) {
        delete [] phartdata_;
    }
}

void DmiFunctional::postinitService() {
    ibus_ = static_cast<IMemoryOperation *>(
            RISCV_get_service_iface(sysbus_.to_string(),
                                    IFACE_MEMORY_OPERATION));
    if (!ibus_) {
        RISCV_error("Can get interace IMemoryOperation from %s",
                    sysbus_.to_string());
    }


    phartdata_ = new HartDataType[cpumax_.to_int()];
    memset(phartdata_, 0, cpumax_.to_int() * sizeof(HartDataType));

    for (unsigned i = 0; i < hartlist_.size(); i++) {
        AttributeType &hart = hartlist_[i];
        phartdata_[i].idport = static_cast<IDPort *>(
                RISCV_get_service_iface(hart.to_string(),
                                        IFACE_DPORT));
        if (!phartdata_[i].idport) {
            RISCV_error("Can get interace IDPort from %s",
                        hart.to_string());
        }
    }
}

ETransStatus DmiFunctional::b_transport(Axi4TransactionType *trans) {
    uint32_t off = static_cast<uint32_t>(trans->addr - getBaseAddress()) >> 2;
    if (trans->xsize != 4 || (trans->addr & 0x3) != 0) {
        return TRANS_ERROR;
    }
    if (trans->action == MemAction_Write) {
        dmi_write(off, trans->wpayload.b32[0]);
    } else {
        dmi_read(off, &trans->rpayload.b32[0]);
    }
    return TRANS_OK;
}

void DmiFunctional::dmi_read(uint32_t addr, uint32_t *rdata) {
    if (addr == IJtag::DMI_DMCONTROL) {
        IJtag::dmi_dmcontrol_type dmcontrol;
        dmcontrol.u32 = 0;
        dmcontrol.bits.dmactive = 1;
        dmcontrol.bits.ndmreset = ndmreset_;
        dmcontrol.bits.hartselhi = hartsel_ >> 10;
        dmcontrol.bits.hartsello = hartsel_;
        *rdata = dmcontrol.u32;
    } else if (addr == IJtag::DMI_DMSTATUS) {
        IJtag::dmi_dmstatus_type dmstatus;
        dmstatus.u32 = 0;
        dmstatus.bits.version = 2;  // 2=ver 0.13; 3=ver 1.0
        dmstatus.bits.authenticated = 1;
        dmstatus.bits.hasresethalreq = 1;
        dmstatus.bits.allhalted = 1;
        dmstatus.bits.anyhalted = 0;
        dmstatus.bits.allrunning = 1;
        dmstatus.bits.anyrunning = 0;
        dmstatus.bits.allresumeack = 1;
        dmstatus.bits.anyresumeack = 0;
        for (unsigned i = 0; i < hartlist_.size(); i++) {
            if (phartdata_[i].idport->isHalted()) {
                dmstatus.bits.allrunning = 0;
                dmstatus.bits.allhalted = 1;
            } else {
                dmstatus.bits.allhalted = 0;
                dmstatus.bits.anyrunning = 1;
            }
            if (phartdata_[i].idport->isResumeAck() == 0) {
                dmstatus.bits.allresumeack = 0;
            } else {
                dmstatus.bits.anyresumeack = 1;
            }
        }

        *rdata = dmstatus.u32;
    } else if (addr == IJtag::DMI_HARTINFO) {
        *rdata = 0;
        *rdata |= (2 << 20);// nscratch
    } else if (addr == IJtag::DMI_ABSTRACTCS) {
        IJtag::dmi_abstractcs_type abstractcs;
        abstractcs.u32 = 0;
        abstractcs.bits.datacount = 2;
        abstractcs.bits.progbufsize = 16;
        abstractcs.bits.cmderr = getCmdErr();
        *rdata = abstractcs.u32;
    } else if (addr == IJtag::DMI_ABSTRACT_DATA0) {
        *rdata = arg0_.buf32[0];
        if (autoexecdata_ & (0x1 << 0)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA1) {
        *rdata = arg0_.buf32[1];
        if (autoexecdata_ & (0x1 << 1)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA2) {
        *rdata = arg1_.buf32[0];
        if (autoexecdata_ & (0x1 << 2)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA3) {
        *rdata = arg1_.buf32[1];
        if (autoexecdata_ & (0x1 << 3)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA4) {
        *rdata = arg2_.buf32[0];
        if (autoexecdata_ & (0x1 << 4)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA5) {
        *rdata = arg2_.buf32[1];
        if (autoexecdata_ & (0x1 << 5)) {
            executeCommand();
        }
    } else if (addr >= IJtag::DMI_PROGBUF0 && 
               addr < (IJtag::DMI_PROGBUF0 + progbufTotal_.to_uint32())) {
        *rdata = progbuf_[addr - IJtag::DMI_PROGBUF0];
    } else if (addr == IJtag::DMI_ABSTRACTAUTO) {
        *rdata = autoexecdata_ | (autoexecprogbuf_ << 16);
    } else if (addr == IJtag::DMI_HALTSUM0) {
        *rdata = 0;
        for (unsigned i = 0; i < hartlist_.size(); i++) {
            if (phartdata_[i].idport->isHalted()) {
                *rdata |= 1u << i;
            }
        }
    } else if (addr == IJtag::DMI_SBCS) {
        *rdata = 0;
    } else {
        RISCV_info("Unimplemented DMI read request at %02x", addr);
    }
}

void DmiFunctional::dmi_write(uint32_t addr, uint32_t wdata) {
    if (addr == IJtag::DMI_DMCONTROL) {
        IDPort *idport;
        IJtag::dmi_dmcontrol_type dmcontrol;
        int errcode = 0;
        dmcontrol.u32 = wdata;
        hartsel_ = dmcontrol.bits.hartselhi;
        hartsel_ = (hartsel_ << 10) | dmcontrol.bits.hartsello;
        if (hartsel_ >= hartlist_.size()) {
            hartsel_ = hartlist_.size() - 1;
        }
        idport = phartdata_[hartsel_].idport;
        if (getCmdErr()) {
            RISCV_info("Cannot execute DMI, cmderr is non-zero: %02x", getCmdErr());
        } else if (dmcontrol.bits.haltreq) {
            if (ndmreset_ == 0 && dmcontrol.bits.ndmreset == 0) {
                errcode = idport->haltreq();
            }
        } else if (dmcontrol.bits.resumereq) {
            errcode = idport->resumereq();
        }

        // Reset negative edge:
        if (ndmreset_ && dmcontrol.bits.ndmreset == 0) {
            // TODO through dedicated reset service
            IResetListener *irst;
            AttributeType rstlist;
            RISCV_get_iface_list(IFACE_RESET_LISTENER, &rstlist);
            for (unsigned i = 0; i < rstlist.size(); i++) {
                irst = static_cast<IResetListener *>(rstlist[i].to_iface());
                irst->reset(static_cast<IService *>(this));
            }
        }
        ndmreset_ = dmcontrol.bits.ndmreset;    // must be after haltreq to avoid cmderr "already halted"

        if (errcode) {
            setCmdErr(IJtag::DMI_ABSTRACTCS_CMDERR_HALTRESUME);
        }
    } else if (addr == IJtag::DMI_ABSTRACTCS) {
        IJtag::dmi_abstractcs_type abstractcs;
        abstractcs.u32 = wdata;
        if (abstractcs.bits.cmderr == 1) {
            setCmdErr(IJtag::DMI_ABSTRACTCS_CMDERR_NONE);
        }
    } else if (addr == IJtag::DMI_COMMAND) {
        command_.u32 = wdata;
        executeCommand();
    } else if (addr == IJtag::DMI_ABSTRACT_DATA0) {
        arg0_.buf32[0] = wdata;
        if (autoexecdata_ & (0x1 << 0)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA1) {
        arg0_.buf32[1] = wdata;
        if (autoexecdata_ & (0x1 << 1)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA2) {
        arg1_.buf32[0] = wdata;
        if (autoexecdata_ & (0x1 << 2)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA3) {
        arg1_.buf32[1] = wdata;
        if (autoexecdata_ & (0x1 << 3)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA4) {
        arg2_.buf32[0] = wdata;
        if (autoexecdata_ & (0x1 << 4)) {
            executeCommand();
        }
    } else if (addr == IJtag::DMI_ABSTRACT_DATA5) {
        arg2_.buf32[1] = wdata;
        if (autoexecdata_ & (0x1 << 5)) {
            executeCommand();
        }
    } else if (addr >= IJtag::DMI_PROGBUF0 && 
               addr < (IJtag::DMI_PROGBUF0 + progbufTotal_.to_uint32())) {
        progbuf_[addr - IJtag::DMI_PROGBUF0] = wdata;
    } else if (addr == IJtag::DMI_ABSTRACTAUTO) {
        autoexecdata_ = wdata & ((1ul << dataregTotal_.to_int()) - 1);
        autoexecprogbuf_ = (wdata >> 16) & ((1ul << progbufTotal_.to_int()) - 1);
    } else {
        RISCV_info("Unimplemented DMI write request at %02x", addr);
    }
}

void DmiFunctional::executeCommand() {
    IDPort *idport = phartdata_[hartsel_].idport;
    if (getCmdErr() != IJtag::DMI_ABSTRACTCS_CMDERR_NONE) {
        RISCV_info("Cannot execute DMI, cmderr is non-zero: %02x", getCmdErr());
        return;
    }

    if (command_.regaccess.cmdtype == 0) {
        int errcode = 0;
        if (command_.regaccess.transfer) {
            if (command_.regaccess.write) {
                errcode = idport->dportWriteReg(command_.regaccess.regno, arg0_.val);
            } else {
                errcode = idport->dportReadReg(command_.regaccess.regno, &arg0_.val);
            }
        }
        if (errcode) {
            setCmdErr(IJtag::DMI_ABSTRACTCS_CMDERR_EXCEPTION);
            RISCV_info("Exception on access to register: %04x", command_.regaccess.regno);
        } else if (command_.regaccess.postexec) {
            errcode = idport->executeProgbuf(progbuf_);
        }
        if (errcode) {
            setCmdErr(IJtag::DMI_ABSTRACTCS_CMDERR_EXCEPTION);
            RISCV_info("Exception on exec. probug, cmderr is non-zero: %02x", getCmdErr());
        } else if (command_.regaccess.aarpostincrement) {
            command_.regaccess.regno++;
        }
    } else if (command_.quickaccess.cmdtype == 1) {
    } else if (command_.memaccess.cmdtype == 2) {
        uint32_t memsz = 1 << command_.memaccess.aamsize;
        int errcode;
        if (command_.memaccess.write) {
            errcode = idport->dportWriteMem(arg1_.val,
                    command_.memaccess.aamvirtual, memsz, arg0_.val);
        } else {
            errcode = idport->dportReadMem(arg1_.val,
                    command_.memaccess.aamvirtual, memsz, &arg0_.val);
        }
        if (errcode) {
            setCmdErr(IJtag::DMI_ABSTRACTCS_CMDERR_EXCEPTION);
            RISCV_info("Exception on memory access to %08x", arg1_.val);
        } else if (command_.memaccess.aampostincrement) {
            arg1_.val += memsz;
        }
    } else {
        RISCV_info("Wrong command type at %02x", command_.regaccess.cmdtype);
    }
}

}  // namespace debugger

