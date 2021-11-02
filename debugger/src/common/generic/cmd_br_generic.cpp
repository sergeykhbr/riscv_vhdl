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
#include "cmd_br_generic.h"
#include "debug/dsumap.h"
#include "debug/dmi_regs.h"

namespace debugger {

CmdBrGeneric::CmdBrGeneric(uint64_t dmibar, ITap *tap) :
    ICommand ("br", dmibar, tap),
    IHap(HAP_Halt) {

    briefDescr_.make_string("Add or remove breakpoint.");
    detailedDescr_.make_string(
        "Description:\n"
        "    Get breakpoints list or add/remove breakpoint with specified\n"
        "    flags.\n"
        "Response:\n"
        "    List of lists [[iii]*] if breakpoint list was requested, where:\n"
        "        i|s  - uint64_t address value or 'string' symbol name\n"
        "        i    - uint32_t instruction value\n"
        "        i    - uint64_t Breakpoint flags: hardware,...\n"
        "    Nil in a case of add/rm breakpoint\n"
        "Usage:\n"
        "    br"
        "    br add <addr>\n"
        "    br rm <addr>\n"
        "    br rm 'symbol_name'\n"
        "    br add <addr> hw\n"
        "Example:\n"
        "    br add 0x10000000\n"
        "    br add 0x00020040 hw\n"
        "    br add 'func1'\n"
        "    br rm 0x10000000\n"
        "    br rm 'func1'\n");

    AttributeType lstServ;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &lstServ);
    isrc_ = 0;
    if (lstServ.size() != 0) {
        IService *iserv = static_cast<IService *>(lstServ[0u].to_iface());
        isrc_ = static_cast<ISourceCode *>(
                            iserv->getInterface(IFACE_SOURCE_CODE));
    }

    RISCV_event_create(&eventHalted_, "CmDrGeneric_halted");
}

CmdBrGeneric::~CmdBrGeneric() {
    RISCV_event_close(&eventHalted_);
}

int CmdBrGeneric::isValid(AttributeType *args) {
    if (!cmdName_.is_equal((*args)[0u].to_string())) {
        return CMD_INVALID;
    }
    if (args->size() == 1 || (args->size() >= 3 && (*args)[1].is_string())) {
        return CMD_VALID;
    }
    return CMD_WRONG_ARGS;
}

void CmdBrGeneric::exec(AttributeType *args, AttributeType *res) {
    res->attr_free();
    res->make_nil();
    if (!isrc_) {
        generateError(res, "ISource interface not found");
        return;
    }
    if (args->size() == 1) {
        isrc_->getBreakpointList(res);
        return;
    }

    uint64_t flags = 0;
    if (args->size() == 4 && (*args)[3].is_equal("hw")) {
        flags |= BreakFlag_HW;
    }

    //CrGenericRuncontrolType runctrl;
        
    //uint64_t addr_dmcontrol = -1;//DSUREGBASE(csr[CSR_runcontrol]);
    Reg64Type braddr;
    AttributeType &bpadr = (*args)[2];
    if (bpadr.is_integer()) {
        braddr.val = bpadr.to_uint64();
    } else if (bpadr.is_string()) {
        if (isrc_->symbol2Address(bpadr.to_string(), &braddr.val) < 0) {
            generateError(res, "Symbol not found");
            return;
        }
    } else {
        generateError(res, "Wrong command format");
        return;
    }

    // Add/remove breakpoints only when CPU is halted
    bool resume = false;
    RISCV_event_clear(&eventHalted_);
    if (!isHalted()) {
        //runctrl.val = 0;
        //runctrl.bits.req_halt = 1;
        //tap_->write(addr_dmcontrol, 8, runctrl.u8);
        
        RISCV_event_wait(&eventHalted_);
        resume = true;
    }
    
    // Update breakpoints list
    if ((*args)[1].is_equal("add")) {
        Reg64Type memdata;
        Reg64Type brdata;
        uint32_t brlen;

        memdata.val = 0;
        tap_->read(braddr.val, 4, memdata.buf);

        brdata = memdata;
        getSwBreakpointInstr(&brdata, &brlen);

        isrc_->registerBreakpoint(braddr.val,
                                  flags,
                                  memdata.buf32[0],
                                  brdata.buf32[0],
                                  brlen);
    } else if ((*args)[1].is_equal("rm")) {
        isrc_->unregisterBreakpoint(braddr.val);
    }

    // Continue execution if cpu was running
    if (resume) {
        RISCV_trigger_hap(HAP_Resume, 0, "Resume command received");

        //runctrl.val = 0;
        //runctrl.bits.req_resume = 1;
        //tap_->write(addr_dmcontrol, 8, runctrl.u8);
    }
}

bool CmdBrGeneric::isHalted() {
    DMSTATUS_TYPE::ValueType dmstatus;
    uint64_t addr_dmstatus = DSUREGBASE(ulocal.v.dmstatus);
    tap_->read(addr_dmstatus, 8, dmstatus.u8);
    return dmstatus.bits.allhalted ? true: false;
}

}  // namespace debugger
