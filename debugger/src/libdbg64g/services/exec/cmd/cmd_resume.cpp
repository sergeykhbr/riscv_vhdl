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

#include <ihap.h>
#include <iservice.h>
#include <riscv-isa.h>
#include "cmd_resume.h"

namespace debugger {

CmdResume::CmdResume(IService *parent, IJtag *ijtag)
    : ICommandRiscv(parent, "resume", ijtag) {

    briefDescr_.make_string("Run simulation for a specify number of steps\"");
    detailedDescr_.make_string(
        "Description:\n"
        "    Run simulation for a specified number of steps.\n"
        "Usage:\n"
        "    resume <N steps>\n"
        "Example:\n"
        "    resume\n"
        "    run\n"
        "    go 1000\n"
        "    c 1\n");

    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &lstServ);
    isrc_ = 0;
    if (lstServ.size() != 0) {
        IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
        isrc_ = static_cast<ISourceCode *>(
                            iserv->getInterface(IFACE_SOURCE_CODE));
    }
}


int CmdResume::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (!cmdName_.is_equal(name.to_string())
        && !name.is_equal("run")
        && !name.is_equal("c")
        && !name.is_equal("go")) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || args->size() == 2) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdResume::exec(AttributeType *args, AttributeType *res) {
    IJtag::dmi_dmstatus_type dmstatus;
    IJtag::dmi_dmcontrol_type dmcontrol;
    IJtag::dmi_command_type command;
    IJtag::dmi_abstractcs_type abstractcs;
    csr_dcsr_type dcsr;
    int watchdog = 0;
    uint32_t stepcnt = 0;
    res->attr_free();
    res->make_nil();

    dmstatus.u32 = read_dmi(IJtag::DMI_DMSTATUS);
    if (dmstatus.bits.allhalted == 0) {
        generateError(res, "Already running");
        return;
    }

    if (args->size() == 2 && (*args)[1].is_integer()) {
        stepcnt = (*args)[1].to_uint32();
    }

    // Write breakpoints and do other stuffs
    RISCV_trigger_hap(HAP_Resume, 0, "Resume command received");

    // Flush everything:
    write_dmi(IJtag::DMI_PROGBUF0, OPCODE_FENCE_I);
    write_dmi(IJtag::DMI_PROGBUF1, OPCODE_FENCE);
    write_dmi(IJtag::DMI_PROGBUF2, OPCODE_EBREAK);

    command.u32 = 0;
    command.regaccess.cmdtype = 0;
    command.regaccess.aarsize = IJtag::CMD_AAxSIZE_32BITS;
    command.regaccess.postexec = 1;
    command.regaccess.regno = reg2addr("r0");
    write_dmi(IJtag::DMI_COMMAND, command.u32);
    wait_dmi();

    // Read and write back the CSR register DCSR with the bits: ebreakm, ebreaks, ebreaku:
    command.u32 = 0;
    command.regaccess.cmdtype = 0;
    command.regaccess.aarsize = IJtag::CMD_AAxSIZE_64BITS;
    command.regaccess.transfer = 1;
    command.regaccess.regno = reg2addr("dcsr");
    write_dmi(IJtag::DMI_COMMAND, command.u32);
    wait_dmi();

    dcsr.u32[0] = read_dmi(IJtag::DMI_ABSTRACT_DATA0);
    dcsr.u32[1] = read_dmi(IJtag::DMI_ABSTRACT_DATA1);
    dcsr.bits.ebreakm = 1;
    dcsr.bits.ebreaks = 1;
    dcsr.bits.ebreaku = 1;
    dcsr.bits.step = 0;
    if (stepcnt) {
        dcsr.bits.step = 1;
    }
    write_dmi(IJtag::DMI_ABSTRACT_DATA0, dcsr.u32[0]);

    command.u32 = 0;
    command.regaccess.cmdtype = 0;
    command.regaccess.aarsize = IJtag::CMD_AAxSIZE_64BITS;
    command.regaccess.transfer = 1;
    command.regaccess.write = 1;
    command.regaccess.regno = reg2addr("dcsr");
    write_dmi(IJtag::DMI_COMMAND, command.u32);
    wait_dmi();


    // set resume request:
    do {
        dmcontrol.u32 = 0;
        dmcontrol.bits.dmactive = 1;
        dmcontrol.bits.resumereq = 1;
        write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);

        // All harts should run:
        dmstatus.u32 = 0;
        do {
            dmstatus.u32 = read_dmi(IJtag::DMI_DMSTATUS);
        } while (dmstatus.bits.allresumeack == 0 && watchdog++ < 5);

        if (dmstatus.bits.allresumeack == 0 && dmstatus.bits.allrunning == 1) {
            generateError(res, "All harts are already running");
            break;
        }

        if (stepcnt) {
            stepcnt--;
        }
    } while (stepcnt);

    // clear resume request
    dmcontrol.u32 = 0;
    dmcontrol.bits.dmactive = 1;
    write_dmi(IJtag::DMI_DMCONTROL, dmcontrol.u32);

    abstractcs.u32 = read_dmi(IJtag::DMI_ABSTRACTCS);
    if (abstractcs.bits.cmderr) {
        clear_cmderr();
    }
}

#if 0
uint64_t CmdResume::checkSwBreakpoint() {
    uint64_t addr_dpc = DSUREGBASE(csr[ICpuRiscV::CSR_dpc]);
    //uint64_t addr_dcsr = DSUREGBASE(csr[CSR_dcsr]);
    //uint64_t addr_runcontrol = -1;//DSUREGBASE(csr[CSR_runcontrol]);
    uint64_t addr_dmstatus = DSUREGBASE(ulocal.v.dmstatus);
    Reg64Type br_addr;
    Reg64Type dpc;
    //CrGenericRuncontrolType runctrl;
    //CrGenericDebugControlType dcsr;
    DMSTATUS_TYPE::ValueType dmstatus;
    Reg64Type steps;
    steps.val = 0;

    tap_->read(addr_dpc, 8, dpc.buf);

    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        br_addr.val = br[BrkList_address].to_uint64();
        if (br_addr.val == dpc.val) {
            // Make step while no breakpoints loaded
            //uint64_t addr_step_cnt = DSUREGBASE(csr[CSR_insperstep]);
            //steps.val = 1;
            //tap_->write(addr_step_cnt, 8, steps.buf);

            // Enable stepping
//            dcsr.val = 0;
//            dcsr.bits.ebreakm = 1;
//            dcsr.bits.step = 1;
//            tap_->write(addr_dcsr, 8, dcsr.u8);

            // resumereq to make step
//            runctrl.val = 0;
//            runctrl.bits.req_resume = 1;
//            tap_->write(addr_runcontrol, 8, runctrl.u8);

            // Wait while hart is running (for low speed simulation)
            do {
                tap_->read(addr_dmstatus, 8, dmstatus.u8);
            } while (!dmstatus.bits.allhalted);
            break;
        }
    }
    return steps.val;
}

void CmdResume::writeBreakpoints() {
    uint64_t br_addr;
    uint64_t br_flags;
    uint32_t br_oplen;
    uint64_t addr_flushi = DSUREGBASE(csr[ICpuRiscV::CSR_flushi]);
    Reg64Type data;

    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        br_addr = br[BrkList_address].to_uint64();
        br_flags = br[BrkList_flags].to_uint64();
        br_oplen = br[BrkList_oplen].to_uint32();
        data.val = br[BrkList_opcode].to_uint32();

        if (br_flags & BreakFlag_HW) {
            // TODO: triggers
        } else {
            tap_->write(br_addr, br_oplen, data.buf);

            // We can execute FENCE.I from progbuf at the end but it is easier
            // to use custom CSR_flushi to clear specific address in I$
            data.val = br_addr;
            tap_->write(addr_flushi, 8, data.buf);
        }
    }
}
#endif

}  // namespace debugger
