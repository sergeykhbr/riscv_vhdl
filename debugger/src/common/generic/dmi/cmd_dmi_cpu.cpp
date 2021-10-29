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

#include "debug/dmi_regs.h"
#include "cmd_dmi_cpu.h"
#include <riscv-isa.h>

namespace debugger {

CmdDmiCpuGneric::CmdDmiCpuGneric(IFace *parent, uint64_t dmibar, ITap *tap)
    : ICommand(static_cast<IService *>(parent)->getObjName(), dmibar, tap) {

    briefDescr_.make_string("Core run control command");
    detailedDescr_.make_string(
        "Description:\n"
        "    Run, halt or run with enabled stepping command for a selected CPU.\n"
        "Example:\n"
        "    <objname> command\n"
        "    Commands:\n"
        "       halt, stop, break - are the commands to stop the CPU\n"
        "       go, run, c - are the command to start the CPU\n"
        "       step - execute one instruction and return into Debug Mode\n"
        "Example:\n"
        "    core0 halt\n"
        "    core0 stop\n"
        "    core0 go\n"
        "    core0 step\n");
}


int CmdDmiCpuGneric::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (name.is_equal("status")) {
        return CMD_VALID;
    }
    if (!cmdName_.is_equal(name.to_string())) {
        return CMD_INVALID;
    }
    if (args->size() < 2) {
        return CMD_WRONG_ARGS;
    }
    AttributeType &par1 = (*args)[1];
    if (!par1.is_equal("halt")
        && !par1.is_equal("stop")
        && !par1.is_equal("break")
        && !par1.is_equal("go")
        && !par1.is_equal("run")
        && !par1.is_equal("c")
        && !par1.is_equal("step")
        && !(par1.is_equal("reg") && (args->size() == 3 || args->size() == 4))
        && !(par1.is_equal("regs") && args->size() >= 3)) {
        return CMD_WRONG_ARGS;
    }
    return CMD_VALID;
}

void CmdDmiCpuGneric::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    AttributeType &name = (*args)[0u];
    AttributeType &par1 = (*args)[1];
    Reg64Type reg = {0};

    if (name.is_equal("status")) {
        // Read arg0
        dma_read(dmibar_+ 4*0x40, 4, reg.buf);    // haltsum0
        res->make_uint64(reg.val);
        return;
    }

    clearcmderr();
    if (par1.is_equal("halt") || par1.is_equal("stop") || par1.is_equal("break")) {
        halt();
        waithalted();
    } else if (par1.is_equal("go") || par1.is_equal("run") || par1.is_equal("c")) {
        resume();
    } else if (par1.is_equal("step")) {
        setStep(1);
        resume();
        waithalted();
        setStep(0);
    } else if (par1.is_equal("regs")) {
        uint32_t addr;
        res->make_list(args->size() - 2);
        for (unsigned i = 2; i < args->size(); i++) {
            AttributeType &regname = (*args)[i];
            addr = reg2addr(regname.to_string());
            if (addr == REG_ADDR_ERROR) {
                continue;
            }
            readreg(addr, reg.buf);
            (*res)[i-2].make_uint64(reg.val);
        }
    } else if (par1.is_equal("reg")) {
        AttributeType &regname = (*args)[2];
        uint32_t addr = reg2addr(regname.to_string());
        if (addr != REG_ADDR_ERROR) {
            if (args->size() == 3) {
                readreg(addr, reg.buf);
            } else if (args->size() == 4) {
                reg.val = (*args)[3].to_uint64();
                writereg(addr, reg.buf);
            }
        }
        res->make_uint64(reg.val);
    }
}

const uint32_t CmdDmiCpuGneric::reg2addr(const char *name) {
    const ECpuRegMapping  *preg = getpMappedReg();
    while (preg->name[0]) {
        if (strcmp(name, preg->name) == 0) {
            return preg->offset;
        }
        preg++;
    }
    return REG_ADDR_ERROR;
}

void CmdDmiCpuGneric::setStep(bool val) {
    // Write data0
    DCSR_TYPE::ValueType dcsr;
    dcsr.val = 0;
    dcsr.bits.stoptime = 1;
    dcsr.bits.stopcount = 1;
    dcsr.bits.ebreakm = 1;
    dcsr.bits.step = val;
    dma_write(dmibar_ + 4*0x04, 4, dcsr.u8);            // arg0[31:0] = data0

    // transfer dcsr,arg0
    COMMAND_TYPE::ValueType command;
    command.val = 0;
    command.bits.transfer = 1;
    command.bits.write = 1;
    command.bits.aarsize = 2;
    command.bits.regno = 0x7b0;//CSR_dcsr;
    dma_write(dmibar_ + 4*0x17, 4, command.u8);
}

void CmdDmiCpuGneric::clearcmderr() {
    ABSTRACTCS_TYPE::ValueType abstractcs;
    abstractcs.val = 0;
    abstractcs.bits.cmderr = 1;
    dma_write(dmibar_ + 4*0x16, 4, abstractcs.u8);
}

void CmdDmiCpuGneric::resume() {
    DMCONTROL_TYPE::ValueType dmcontrol;
    DMSTATUS_TYPE::ValueType dmstatus;
    dmcontrol.val = 0;
    dmcontrol.bits.resumereq = 1;
    dma_write(dmibar_ + 4*0x10, 4, dmcontrol.u8);
    // Wait until resume request accepted
    bool stepped = false;
    while (!stepped) {
        dma_read(dmibar_ + 0x11*0x4, 4, dmstatus.u8);
        if (dmstatus.bits.allresumeack) {
            stepped = true;
        }
    }
}

void CmdDmiCpuGneric::halt() {
    DMCONTROL_TYPE::ValueType dmcontrol;
    dmcontrol.val = 0;
    dmcontrol.bits.haltreq = 1;
    dma_write(dmibar_ + 4*0x10, 4, dmcontrol.u8);
}

void CmdDmiCpuGneric::waithalted() {
    DMSTATUS_TYPE::ValueType dmstatus;
    bool finish = false;
    while (!finish) {
        dma_read(dmibar_ + 0x11*0x4, 4, dmstatus.u8);
        if (dmstatus.bits.allhalted) {
            finish = true;
        }
    }
}

void CmdDmiCpuGneric::readreg(uint32_t regno, uint8_t *buf8) {
    COMMAND_TYPE::ValueType command;
    command.val = 0;
    command.bits.transfer = 1;
    command.bits.aarsize = 3;
    command.bits.regno = regno;
    dma_write(dmibar_ + 4*0x17, 4, command.u8);     // arg0 = [data1,data0]
    // Read arg0
    dma_read(dmibar_ + 4*0x4, 4, buf8);          // [data1,data0]
    dma_read(dmibar_ + 4*0x5, 4, &buf8[4]);
}

void CmdDmiCpuGneric::writereg(uint32_t regno, uint8_t *buf8) {
    COMMAND_TYPE::ValueType command;
    // Read arg0
    dma_write(dmibar_ + 4*0x4, 4, buf8);          // [data1,data0]
    dma_write(dmibar_ + 4*0x5, 4, &buf8[4]);
    command.val = 0;
    command.bits.transfer = 1;
    command.bits.write = 1;
    command.bits.aarsize = 3;
    command.bits.regno = regno;
    dma_write(dmibar_ + 4*0x17, 4, command.u8);     // arg0 = [data1,data0]
}

const ECpuRegMapping *CmdDmiCpuRiscV::getpMappedReg() {
    return RISCV_DEBUG_REG_MAP;
}

}  // namespace debugger
