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

#include <generic-isa.h>
#include <ihap.h>
#include <iservice.h>
#include "cmd_run.h"
#include "debug/dsumap.h"
#include "debug/dmi_regs.h"

namespace debugger {

CmdRun::CmdRun(ITap *tap) : ICommand ("run", tap) {

    briefDescr_.make_string("Run simulation for a specify number of steps\"");
    detailedDescr_.make_string(
        "Description:\n"
        "    Run simulation for a specified number of steps.\n"
        "Usage:\n"
        "    run <N steps>\n"
        "Example:\n"
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


int CmdRun::isValid(AttributeType *args) {
    AttributeType &name = (*args)[0u];
    if (!cmdName_.is_equal(name.to_string())
        && !name.is_equal("c")
        && !name.is_equal("go")) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || args->size() == 2) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdRun::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    CrGenericRuncontrolType runctrl;
    CrGenericDebugControlType dcs;
    uint64_t addr_runcontrol = DSUREGBASE(csr[CSR_runcontrol]);
    uint64_t addr_dcsr = DSUREGBASE(csr[CSR_dcsr]);
    //uint64_t addr_step_cnt = DSUREGBASE(csr[CSR_insperstep]);
    uint64_t steps_skipped = 0;

    isrc_->getBreakpointList(&brList_);
    if (brList_.size()) {
        // Skip breakpoint if npc points to ebreak
        steps_skipped = checkSwBreakpoint();
        // Write all breakpoints before resuming
        writeBreakpoints();
    }

    if (args->size() == 2) {
        Reg64Type step_cnt;
        step_cnt.val = (*args)[1].to_uint64() - steps_skipped;
        if (step_cnt.val)  {
            //tap_->write(addr_step_cnt, 8, step_cnt.buf);

            dcs.val = 0;                // disable step mode
            dcs.bits.step = 1;
            dcs.bits.ebreakm = 1;       // openocd do the same for m,h,s,u
            tap_->write(addr_dcsr, 8, dcs.u8);

            runctrl.val = 0;
            runctrl.bits.req_resume = 1;
            tap_->write(addr_runcontrol, 8, runctrl.u8);
        }
    } else {
        runctrl.val = 0;
        runctrl.bits.req_resume = 1;
        tap_->write(addr_runcontrol, 8, runctrl.u8);
    }

    RISCV_trigger_hap(HAP_Resume, 0, "Resume command processed");
}

uint64_t CmdRun::checkSwBreakpoint() {
    uint64_t addr_dpc = DSUREGBASE(csr[CSR_dpc]);
    uint64_t addr_dcsr = DSUREGBASE(csr[CSR_dcsr]);
    uint64_t addr_runcontrol = DSUREGBASE(csr[CSR_runcontrol]);
    uint64_t addr_dmstatus = DSUREGBASE(ulocal.v.dmstatus);
    Reg64Type br_addr;
    Reg64Type dpc;
    CrGenericRuncontrolType runctrl;
    CrGenericDebugControlType dcsr;
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
            dcsr.val = 0;
            dcsr.bits.ebreakm = 1;
            dcsr.bits.step = 1;
            tap_->write(addr_dcsr, 8, dcsr.u8);

            // resumereq to make step
            runctrl.val = 0;
            runctrl.bits.req_resume = 1;
            tap_->write(addr_runcontrol, 8, runctrl.u8);

            // Wait while hart is running (for low speed simulation)
            do {
                tap_->read(addr_dmstatus, 8, dmstatus.u8);
            } while (!dmstatus.bits.allhalted);
            break;
        }
    }
    return steps.val;
}

void CmdRun::writeBreakpoints() {
    uint64_t br_addr;
    uint64_t br_flags;
    uint32_t br_oplen;
    uint64_t addr_flushi = DSUREGBASE(csr[CSR_flushi]);
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


}  // namespace debugger
