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
    : RegMemBankGeneric(name),
    databuf(this, "databuf", 0x04*sizeof(uint32_t)),
    dmcontrol(this, "dmcontrol", 0x10*sizeof(uint32_t)),
    dmstatus(this, "dmstatus", 0x11*sizeof(uint32_t)),
    hartinfo(this, "hartinfo", 0x12*sizeof(uint32_t)),
    abstractcs(this, "abstractcs", 0x16*sizeof(uint32_t)),
    command(this, "command", 0x17*sizeof(uint32_t)),
    abstractauto(this, "abstractauto", 0x18*sizeof(uint32_t)),
    progbuf(this, "progbuf", 0x20*sizeof(uint32_t)),
    sbcs(this, "sbcs", 0x38*sizeof(uint32_t)),
    haltsum0(this, "haltsum0", 0x40*sizeof(uint32_t)) {

    registerInterface(static_cast<IDmi *>(this));

    registerAttribute("SysBus", &sysbus_);
    registerAttribute("SysBusMasterID", &busid_);
    registerAttribute("CpuMax", &cpumax_);
    registerAttribute("DataregTotal", &dataregTotal_);
    registerAttribute("ProgbufTotal", &progbufTotal_);
    registerAttribute("HartList", &hartlist_);

    phartdata_ = 0;
}

DmiFunctional::~DmiFunctional() {
    if (phartdata_) {
        delete [] phartdata_;
    }
}

void DmiFunctional::postinitService() {
    RegMemBankGeneric::postinitService();

    // Will be mapped with the changed size by hap trigger handler
    databuf.setRegTotal(dataregTotal_.to_int());
    progbuf.setRegTotal(progbufTotal_.to_int());

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

void DmiFunctional::readTransfer(uint32_t regno, uint32_t size) {
    uint32_t region = regno >> 12;
    IDPort *idport = phartdata_[getHartSelected()].idport;
    uint64_t rdata = 0;
    if (region == 0) {
        rdata = idport->readCSR(regno);
    } else if (region == 1) {
        rdata = idport->readGPR(regno & 0x3F);
    } else if (region == 0xc) {
        rdata = idport->readNonStandardReg(regno & 0xFFF);
    }
    switch (size) {
    case 0:
        rdata &= 0xFFull;
        databuf.getp()[0].val = static_cast<uint32_t>(rdata);
        break;
    case 1:
        rdata &= 0xFFFFull;
        databuf.getp()[0].val = static_cast<uint32_t>(rdata);
        break;
    case 2:
        rdata &= 0xFFFFFFFFull;
        databuf.getp()[0].val = static_cast<uint32_t>(rdata);
        break;
    default:
        databuf.getp()[0].val = static_cast<uint32_t>(rdata);
        databuf.getp()[1].val = static_cast<uint32_t>(rdata >> 32);
    }
}

void DmiFunctional::writeTransfer(uint32_t regno, uint32_t size) {
    uint32_t region = regno >> 12;
    IDPort *idport = phartdata_[getHartSelected()].idport;
    uint64_t arg0 = databuf.getp()[1].val;
    arg0 = (arg0 << 32) | databuf.getp()[0].val;
    if (size == 2) {
        arg0 &= 0xFFFFFFFFull;
    }
    if (region == 0) {
        idport->writeCSR(regno, arg0);
    } else if (region == 1) {
        idport->writeGPR(regno & 0x3F, arg0);
    } else if (region == 0xc) {
        idport->writeNonStandardReg(regno & 0xFFF, arg0);
    }
}

}  // namespace debugger

