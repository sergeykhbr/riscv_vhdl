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

#include <api_core.h>
#include "coreservices/isocinfo.h"
#include "cpu_arm7_func.h"

namespace debugger {

CpuCortex_Functional::CpuCortex_Functional(const char *name) :
    CpuGeneric(name),
    portRegs_(this, "regs", 0x8000, Reg_Total),
    portSavedRegs_(this, "savedregs", 0, Reg_Total) {
    registerInterface(static_cast<ICpuArm *>(this));
    registerAttribute("VectorTable", &vectorTable_);
    p_psr_ = reinterpret_cast<ProgramStatusRegsiterType *>(
            &portRegs_.getp()[Reg_cpsr]);
}

CpuCortex_Functional::~CpuCortex_Functional() {
}

void CpuCortex_Functional::postinitService() {
    // Supported instruction sets:
    for (int i = 0; i < INSTR_HASH_TABLE_SIZE; i++) {
        listInstr_[i].make_list(0);
    }
    addArm7tmdiIsa();

    CpuGeneric::postinitService();

    pcmd_br_ = new CmdBrArm(itap_, iinfo_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_br_));
}

void CpuCortex_Functional::predeleteService() {
    CpuGeneric::predeleteService();

    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_br_));
    delete pcmd_br_;
}

void CpuCortex_Functional::hapTriggered(IFace *isrc, EHapType type,
                                        const char *descr) {
    AttributeType srvlist;
    RISCV_get_services_with_iface(IFACE_RESET, &srvlist);
    IService *iserv;
    IReset *irst;
    for (unsigned i = 0; i < srvlist.size(); i++) {
        iserv = static_cast<IService *>(srvlist[i].to_iface());
        irst = static_cast<IReset *>(iserv->getInterface(IFACE_RESET));
        irst->powerOnPressed();
    }

    CpuGeneric::hapTriggered(isrc, type, descr);
}

unsigned CpuCortex_Functional::addSupportedInstruction(
                                    ArmInstruction *instr) {
    AttributeType tmp(instr);
    listInstr_[instr->hash()].add_to_list(&tmp);
    return 0;
}

void CpuCortex_Functional::handleTrap() {
    if (interrupt_pending_ == 0) {
        return;
    }
    if (interrupt_pending_ & (1ull << Interrupt_SoftwareIdx)) {
        DsuMapType::udbg_type::debug_region_type::breakpoint_control_reg t1;
        t1.val = br_control_.getValue().val;
        if (t1.bits.trap_on_break == 0) {
            sw_breakpoint_ = true;
            interrupt_pending_ &= ~(1ull << Interrupt_SoftwareIdx);
            npc_.setValue(pc_.getValue());
            halt("UND Breakpoint");
            return;
        }
    }
    npc_.setValue(0 + 4*0);
    interrupt_pending_ = 0;
}

void CpuCortex_Functional::reset(bool active) {
    CpuGeneric::reset(active);
    portRegs_.reset();
    estate_ = CORE_Halted;
}

GenericInstruction *CpuCortex_Functional::decodeInstruction(Reg64Type *cache) {
    ArmInstruction *instr = NULL;
    int hash_idx = hash32(cacheline_[0].buf32[0]);
    for (unsigned i = 0; i < listInstr_[hash_idx].size(); i++) {
        instr = static_cast<ArmInstruction *>(
                        listInstr_[hash_idx][i].to_iface());
        if (instr->parse(cacheline_[0].buf32)) {
            break;
        }
        instr = NULL;
    }
    portRegs_.getp()[Reg_pc].val = getPC();
    return instr;
}

void CpuCortex_Functional::generateIllegalOpcode() {
    //raiseSignal(EXCEPTION_InstrIllegal);
    RISCV_error("Illegal instruction at 0x%08" RV_PRI64 "x", getPC());
}

void CpuCortex_Functional::trackContextStart() {
    if (reg_trace_file == 0) {
        return;
    }
    /** Save previous reg values to find modification after exec() */
    uint64_t *dst = portSavedRegs_.getpR64();
    uint64_t *src = portRegs_.getpR64();
    memcpy(dst, src, Reg_Total*sizeof(uint64_t));
}

void CpuCortex_Functional::trackContextEnd() {
    if (reg_trace_file == 0) {
        return;
    }
    int sz;
    char tstr[1024];
    const char *pinstrname = "unknown";
    if (instr_) {
        pinstrname = instr_->name();
    }
    sz = RISCV_sprintf(tstr, sizeof(tstr),"%8I64d [%08x]: %8s ",
        step_cnt_, pc_.getValue().buf32[0],
        pinstrname);

    bool reg_changed = false;
    uint64_t *prev = portSavedRegs_.getpR64();
    uint64_t *cur = portRegs_.getpR64();
    for (int i = 0; i < Reg_Total; i++) {
        if (prev[i] != cur[i]) {
            reg_changed = true;
            sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz,
                    "%3s <= %016I64x ",
                    IREGS_NAMES[i], cur[i]);
        }
    }
    if (instr_ && !reg_changed) {
        sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz, "-", NULL);
    }
    (*reg_trace_file) << tstr << "\n";
    reg_trace_file->flush();
}

void CpuCortex_Functional::raiseSignal(int idx) {
    RISCV_error("Raise unsupported signal %d", idx);
}

void CpuCortex_Functional::lowerSignal(int idx) {
    interrupt_pending_ &= ~(1 << idx);
    RISCV_error("Lower unsupported signal %d", idx);
}

void CpuCortex_Functional::raiseSoftwareIrq() {
    interrupt_pending_ |= 1ull << Interrupt_SoftwareIdx;
}

}  // namespace debugger

