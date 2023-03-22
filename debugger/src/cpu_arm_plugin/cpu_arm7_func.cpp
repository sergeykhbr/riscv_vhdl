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
#include "cpu_arm7_func.h"

namespace debugger {

EIsaArmV7 decoder_arm(uint32_t ti, char *errmsg, size_t errsz);
EIsaArmV7 decoder_thumb(uint32_t ti, uint32_t *tio,
                        char *errmsg, size_t errsz);


CpuCortex_Functional::CpuCortex_Functional(const char *name) :
    CpuGeneric(name) {
    registerInterface(static_cast<ICpuArm *>(this));
    registerAttribute("VectorTable", &vectorTable_);
    registerAttribute("DefaultMode", &defaultMode_);
    p_psr_ = reinterpret_cast<ProgramStatusRegsiterType *>(
            &R[Reg_cpsr]);
    PC_ = &R[Reg_pc];   // redefine location of PC register in bank
}

CpuCortex_Functional::~CpuCortex_Functional() {
}

void CpuCortex_Functional::postinitService() {
    // Supported instruction sets:
    for (int i = 0; i < INSTR_HASH_TABLE_SIZE; i++) {
        listInstr_[i].make_list(0);
    }
    addArm7tmdiIsa();
    addThumb2Isa();

    CpuGeneric::postinitService();

    /*pcmd_br_ = new CmdBrArm(dmibar_.to_uint64(), 0);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_br_));

    pcmd_reg_ = new CmdRegArm(dmibar_.to_uint64(), 0);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_reg_));

    pcmd_regs_ = new CmdRegsArm(0);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_regs_));*/

    if (defaultMode_.is_equal("Thumb")) {
        setInstrMode(THUMB_mode);
    }
}

void CpuCortex_Functional::predeleteService() {
    CpuGeneric::predeleteService();

    /*icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_br_));
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_reg_));
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_regs_));
    delete pcmd_br_;
    delete pcmd_reg_;
    delete pcmd_regs_;*/
}

/** HAP_ConfigDone */
void CpuCortex_Functional::hapTriggered(EHapType type,
                                        uint64_t param,
                                        const char *descr) {
    AttributeType pwrlist;
    IPower *ipwr;
    RISCV_get_iface_list(IFACE_POWER, &pwrlist);
    for (unsigned i = 0; i < pwrlist.size(); i++) {
        ipwr = static_cast<IPower *>(pwrlist[i].to_iface());
        ipwr->power(POWER_ON);
    }

    CpuGeneric::hapTriggered(type, param, descr);
}

unsigned CpuCortex_Functional::addSupportedInstruction(
                                    ArmInstruction *instr) {
    AttributeType tmp(instr);
    listInstr_[instr->hash()].add_to_list(&tmp);
    return 0;
}

/*void CpuCortex_Functional::handleTrap() {
    // Check software before checking I-bit
    if (interrupt_pending_[0] & (1ull << Interrupt_SoftwareIdx)) {
        //DCSR_TYPE::ValueType dcsr;
        //dcsr.val = static_cast<uint32_t>(readCSR(CSR_dcsr));
        //if (dcsr.bits.ebreakm == 1) {
            interrupt_pending_[0] &= ~(1ull << Interrupt_SoftwareIdx);
            setNPC(getPC());
            halt(HALT_CAUSE_EBREAK, "SWI Breakpoint");
            return;
        //}
    }

    if (getI() == 1) {
        return;
    }
    if ((interrupt_pending_[0] | interrupt_pending_[1]) == 0) {
        return;
    }
    if (InITBlock()) {
        // To simplify psr control suppose interrupts outside of blocks
        return;
    }

    int irq_idx;
    for (int i = 0; i < 2; i++) {
        if (interrupt_pending_[i] == 0) {
            continue;
        }

        for (int n = 0; n < 64; n++) {
            if ((interrupt_pending_[i] & (1ull << n)) == 0) {
                continue;
            }
            irq_idx = 64*i + n;
            if (irq_idx < 16) {
                enterException(irq_idx);
            } else {
                RISCV_error("Interrupts from NVIC not supported %d", irq_idx);
            }
        }
        interrupt_pending_[i] = 0;
    }
    interrupt_pending_[0] = 0;
}*/

void CpuCortex_Functional::enterException(int idx) {
    // Save register into stack
    trans_.addr = static_cast<uint32_t>(R[Reg_sp]);
    trans_.action = MemAction_Write;
    trans_.wstrb = 0xF;
    trans_.xsize = 4; 

    trans_.addr -= 4;
    trans_.wpayload.b32[0] = static_cast<uint32_t>(R[Reg_r0]);
    dma_memop(&trans_);

    trans_.addr -= 4;
    trans_.wpayload.b32[0] = static_cast<uint32_t>(R[Reg_r1]);
    dma_memop(&trans_);

    trans_.addr -= 4;
    trans_.wpayload.b32[0] = static_cast<uint32_t>(R[Reg_r2]);
    dma_memop(&trans_);

    trans_.addr -= 4;
    trans_.wpayload.b32[0] = static_cast<uint32_t>(R[Reg_r3]);
    dma_memop(&trans_);

    trans_.addr -= 4;
    trans_.wpayload.b32[0] = static_cast<uint32_t>(R[Reg_fe]);
    dma_memop(&trans_);

    trans_.addr -= 4;
    trans_.wpayload.b32[0] = static_cast<uint32_t>(R[Reg_lr]);
    dma_memop(&trans_);

    trans_.addr -= 4;
    trans_.wpayload.b32[0] =
            static_cast<uint32_t>(getNPC() | 0x1);
    dma_memop(&trans_);

    trans_.addr -= 4;
    trans_.wpayload.b32[0] = static_cast<uint32_t>(R[Reg_cpsr]);
    dma_memop(&trans_);

    setReg(Reg_sp, static_cast<uint32_t>(trans_.addr));
    setReg(Reg_lr, 0xFFFFFFF9);     // EXC_RETURN

    // Load vector address:
    trans_.action = MemAction_Read;
    trans_.addr = 4*idx;
    trans_.xsize = 4; 
    trans_.wstrb = 0;
    dma_memop(&trans_);
                
    uint32_t npc = trans_.rpayload.b32[0];
    if (npc & 0x1) {
        setInstrMode(THUMB_mode);
        npc &= ~0x1ul;
    } else {
        setInstrMode(ARM_mode);
    }
    setNPC(npc);
}

void CpuCortex_Functional::exitException(uint32_t exc_return) {
    trans_.action = MemAction_Read;
    trans_.addr = R[Reg_sp];
    trans_.xsize = 4; 
    trans_.wstrb = 0;

    dma_memop(&trans_);
    R[Reg_cpsr] = trans_.rpayload.b32[0];
    trans_.addr += 4;

    dma_memop(&trans_);
    uint32_t npc = trans_.rpayload.b32[0] & ~0x1ul;
    setReg(Reg_pc, npc);
    setBranch(npc);
    trans_.addr += 4;

    dma_memop(&trans_);
    setReg(Reg_lr, trans_.rpayload.b32[0]);
    trans_.addr += 4;

    dma_memop(&trans_);
    setReg(Reg_fe, trans_.rpayload.b32[0]);
    trans_.addr += 4;

    dma_memop(&trans_);
    setReg(Reg_r3, trans_.rpayload.b32[0]);
    trans_.addr += 4;

    dma_memop(&trans_);
    setReg(Reg_r2, trans_.rpayload.b32[0]);
    trans_.addr += 4;

    dma_memop(&trans_);
    setReg(Reg_r1, trans_.rpayload.b32[0]);
    trans_.addr += 4;

    dma_memop(&trans_);
    setReg(Reg_r0, trans_.rpayload.b32[0]);
    trans_.addr += 4;

    setReg(Reg_sp, static_cast<uint32_t>(trans_.addr));
}

uint64_t CpuCortex_Functional::getResetAddress() {
    Axi4TransactionType tr;
    tr.action = MemAction_Read;
    tr.addr = resetVector_.to_uint64() + 4;
    tr.source_idx = sysBusMasterID_.to_int();
    tr.xsize = 4; 
    dma_memop(&tr);
    return tr.rpayload.b32[0] - 1;
}

void CpuCortex_Functional::reset(IFace *isource) {
    Axi4TransactionType tr;
    CpuGeneric::reset(isource);
    ITBlockEnabled = false;
    ITBlockBaseCond_ = 0;
    ITBlockMask_ = 0;
    ITBlockCondition_ = Cond_AL;

    tr.action = MemAction_Read;
    tr.addr = resetVector_.to_uint64();
    tr.source_idx = sysBusMasterID_.to_int();
    tr.xsize = 4; 
    dma_memop(&tr);
    setReg(Reg_sp, tr.rpayload.b32[0]);
    if (defaultMode_.is_equal("Thumb")) {
        setInstrMode(THUMB_mode);
    }
    estate_ = CORE_Halted;
}

void CpuCortex_Functional::StartITBlock(uint32_t firstcond, uint32_t mask) {
    // bf0c  : ite eq => eq ne
    // bf14  : ite ne => ne eq
    // bf16  : itet ne => ne eq ne

    ITBlockBaseCond_ = firstcond;

    uint32_t base0 = firstcond & 0x1;
    uint32_t m3 = (mask >> 3) & 0x1;
    uint32_t m2 = (mask >> 2) & 0x1;
    uint32_t m1 = (mask >> 1) & 0x1;
    
    if ((mask & 0x7) == 0) {
        // One instruction condition
        ITBlockMask_ = (base0 << 4) | 0x8;
    } else if ((mask & 0x3) == 0) {
        // Two instructions condition
        ITBlockMask_ = (base0 << 4) | (m3 << 3) | 0x4;
    } else if ((mask & 0x1) == 0) {
        // Three instructions condition
        ITBlockMask_ = (base0 << 4) | (m3 << 3) | (m2 << 2) | 0x2;
    } else {
        // Four instructions condition
        ITBlockMask_ = (base0 << 4) | (m3 << 3) | (m2 << 2) | (m1 << 2) | 0x1;
    }
    ITBlockCondition_ = firstcond;
    ITBlockEnabled = true;
}

void CpuCortex_Functional::trackContextEnd() {
    CpuGeneric::trackContextEnd();

    if (ITBlockMask_ && !ITBlockEnabled) {
        if (LastInITBlock()) {
            // See table 2-1 shifting of IT execution stage
            ITBlockMask_ = 0;
            ITBlockCondition_ = Cond_AL;
        } else {
            ITBlockMask_ = ITBlockMask_ << 1;  // 5 bits field
            ITBlockCondition_ =
                (ITBlockBaseCond_ & ~0x1) | ((ITBlockMask_ >> 4) & 1);
        }
    }
    ITBlockEnabled = false;     // Just to skip IT instruction
}

GenericInstruction *CpuCortex_Functional::decodeInstruction(Reg64Type *cache) {
    GenericInstruction *instr = NULL;
    uint32_t ti = cacheline_[0].buf32[0];

    EIsaArmV7 etype;
    if (getInstrMode() == THUMB_mode) {
        uint32_t tio;
        etype = decoder_thumb(ti, &tio, errmsg_, sizeof(errmsg_));
        //cacheline_[0].buf32[0] = tio;
    } else {
        etype = decoder_arm(ti, errmsg_, sizeof(errmsg_));
    }

    if (etype < ARMV7_Total) {
        instr = isaTableArmV7_[etype];
    } else {
        RISCV_error("ARM decoder error [%08" RV_PRI64 "x] %08x",
                    getPC(), ti);
    }
    portRegs_.getp()[Reg_pc].val = getPC();
    return instr;
}

void CpuCortex_Functional::generateIllegalOpcode() {
    //raiseSignal(EXCEPTION_InstrIllegal);
    RISCV_error("Illegal instruction at 0x%08" RV_PRI64 "x", getPC());
}

void CpuCortex_Functional::traceOutput() {
    char tstr[1024];
    trace_action_type *pa;

    isrc_->disasm(THUMB_mode,
                trace_data_.pc,
                 &trace_data_.instrbuf,
                 &trace_data_.asmlist);

    RISCV_sprintf(tstr, sizeof(tstr),
        "%9" RV_PRI64 "d: %08" RV_PRI64 "x: %s \n",
            trace_data_.step_cnt - 1,
            trace_data_.pc,
            trace_data_.asmlist[0u].to_string());
    (*trace_file_) << tstr;

    for (int i = 0; i < trace_data_.action_cnt; i++) {
        pa = &trace_data_.action[i];
        if (!pa->memop) {
            RISCV_sprintf(tstr, sizeof(tstr),
                "%21s %10s <= %08x\n",
                    "",
                    ARM_IREGS_NAMES[pa->waddr],
                    static_cast<uint32_t>(pa->wdata));
        } else if (pa->memop_write) {
            RISCV_sprintf(tstr, sizeof(tstr),
                "%21s [%08" RV_PRI64 "x] <= %08x\n",
                    "",
                    pa->memop_addr,
                    pa->memop_data.buf32[0]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr),
                "%21s [%08" RV_PRI64 "x] => %08x\n",
                    "",
                    pa->memop_addr,
                    pa->memop_data.buf32[0]);
        }
        (*trace_file_) << tstr;
    }

    trace_file_->flush();
}

void CpuCortex_Functional::raiseSignal(int idx) {
    if (idx >= 128) {
        RISCV_error("Raise unsupported signal %d", idx);
    }
    RISCV_debug("Request Interrupt %d", idx);
    interrupt_pending_[idx >> 6] |= (1ull << (idx & 0x3F));
}

void CpuCortex_Functional::lowerSignal(int idx) {
    interrupt_pending_[idx >> 6] &= ~(1ull << (idx & 0x3F));
    RISCV_error("Lower unsupported signal %d", idx);
}

void CpuCortex_Functional::raiseSoftwareIrq() {
    interrupt_pending_[0] |= (1ull << Interrupt_SoftwareIdx);
}

}  // namespace debugger

