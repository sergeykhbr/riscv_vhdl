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
#include "cpu_riscv_func.h"
#include "generic/riscv_disasm.h"
#include "generic/dmi/cmd_dmi_cpu.h"
#include "debug/dmi_regs.h"

namespace debugger {

CpuRiver_Functional::CpuRiver_Functional(const char *name) :
    CpuGeneric(name) {
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerAttribute("VendorID", &vendorid_);
    registerAttribute("ImplementationID", &implementationid_);
    registerAttribute("ContextID", &contextid_);
    registerAttribute("HartID", &hartid_);
    registerAttribute("ListExtISA", &listExtISA_);
    registerAttribute("PLIC", &plic_);

    mmuReservatedAddr_ = 0;
    mmuReservedAddrWatchdog_ = 0;
}

CpuRiver_Functional::~CpuRiver_Functional() {
}

void CpuRiver_Functional::postinitService() {
    // Supported instruction sets:
    for (int i = 0; i < INSTR_HASH_TABLE_SIZE; i++) {
        listInstr_[i].make_list(0);
    }
    addIsaUserRV64I();
    addIsaPrivilegedRV64I();
    for (unsigned i = 0; i < listExtISA_.size(); i++) {
        if (listExtISA_[i].to_string()[0] == 'A') {
            addIsaExtensionA();
        } else if (listExtISA_[i].to_string()[0] == 'C') {
            addIsaExtensionC();
        } else if (listExtISA_[i].to_string()[0] == 'D') {
            addIsaExtensionD();
        } else if (listExtISA_[i].to_string()[0] == 'F') {
            addIsaExtensionF();
        } else if (listExtISA_[i].to_string()[0] == 'M') {
            addIsaExtensionM();
        }
    }

    // Power-on
    reset(0);

    CpuGeneric::postinitService();

    iirq_ = static_cast<IIrqController *>(RISCV_get_service_iface(
        plic_.to_string(), IFACE_IIRQ_CONTROLLER));
    if (!iirq_) {
        RISCV_error("Interface IIrqController in %s not found",
                    plic_.to_string());
    }

    pcmd_br_ = new CmdBrRiscv(dmibar_.to_uint64(), 0);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_br_));

    pcmd_cpu_ = new CmdDmiCpuRiscV(static_cast<IService *>(this));
    pcmd_cpu_->enableDMA(isysbus_, dmibar_.to_uint64());
    icmdexec_->registerCommand(pcmd_cpu_);

}

void CpuRiver_Functional::predeleteService() {
    CpuGeneric::predeleteService();

    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_br_));
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_cpu_));
    delete pcmd_br_;
    delete pcmd_cpu_;
}

unsigned CpuRiver_Functional::addSupportedInstruction(
                                    RiscvInstruction *instr) {
    AttributeType tmp(instr);
    listInstr_[instr->hash()].add_to_list(&tmp);
    return 0;
}

void CpuRiver_Functional::handleTrap() {
    csr_mstatus_type mstatus;
    csr_mcause_type mcause;

    /** Check stack protection exceptions: */
    uint64_t mstackovr = portCSR_.read(CSR_mstackovr).val;
    uint64_t mstackund = portCSR_.read(CSR_mstackund).val;
    uint64_t sp = portRegs_.read(Reg_sp).val;
    if (mstackovr != 0 && sp < mstackovr) {
        raiseSignal(EXCEPTION_StackOverflow);
        portCSR_.write(CSR_mstackovr, 0);
    } else if (mstackund != 0 && sp > mstackund) {
        raiseSignal(EXCEPTION_StackUnderflow);
        portCSR_.write(CSR_mstackund, 0);
    }

    if ((interrupt_pending_[0] | interrupt_pending_[1]) == 0) {
        return;
    }

    mstatus.value = portCSR_.read(CSR_mstatus).val;
    uint64_t exception_mask = (1ull << SIGNAL_XSoftware) - 1;
    if ((interrupt_pending_[0] & exception_mask) == 0 && 
        mstatus.bits.MIE == 0 && cur_prv_level == PRV_M) {
        return;
    }

    int pendidx = -1;
    for (size_t i = 0; i < 8*sizeof(interrupt_pending_); i++) {
        if (interrupt_pending_[i >> 6] & (1ull << (i & 0x3f))) {
            pendidx = static_cast<int>(i);
            break;
        }
    }
    mcause.value     = 0;
    if (pendidx >= EXCEPTIONS_Total) {
        mcause.bits.irq  = 1;
        mcause.bits.code = pendidx - EXCEPTIONS_Total + PRV_M;
    } else {
        mcause.bits.code = pendidx;
    }
    if (estate_ != CORE_ProgbufExec) {
        portCSR_.write(CSR_mcause, mcause.value);
    }

    if (mcause.bits.irq == 0 && mcause.value == EXCEPTION_Breakpoint) {
        if (estate_ == CORE_ProgbufExec) {
            exitProgbufExec();
            return;
        }

        DCSR_TYPE::ValueType dcsr;
        dcsr.val = static_cast<uint32_t>(readCSR(CSR_dcsr));
        if (dcsr.bits.ebreakm == 1) {
            interrupt_pending_[0] &= ~(1ull << EXCEPTION_Breakpoint);
            setNPC(getPC());
            halt(HALT_CAUSE_EBREAK, "EBREAK Breakpoint");
            return;
        } else {
            // Call ebreak handler
            setNPC(readCSR(CSR_mtvec));
        }
    }

    // All traps handle via machine mode while CSR mdelegate
    // doesn't setup other.
    // @todo delegating
    mstatus.bits.MPP = cur_prv_level;
    mstatus.bits.MPIE = (mstatus.value >> cur_prv_level) & 0x1;
    mstatus.bits.MIE = 0;
    cur_prv_level = PRV_M;
    portCSR_.write(CSR_mstatus, mstatus.value);

    int xepc = static_cast<int>((cur_prv_level << 8) + 0x41);
    portCSR_.write(xepc, getNPC());

    uint64_t mtvec = portCSR_.read(CSR_mtvec).val;
    uint64_t mtvecmode = mtvec & 0x3;
    mtvec &= ~0x3ull;
    if (mtvecmode == 0x1 && mcause.bits.irq) {
        setNPC(mtvec + 4 * mcause.bits.code);
    } else {
        setNPC(mtvec);
    }
    interrupt_pending_[pendidx >> 6] &= ~(1ull << (pendidx & 0x3F));
}

void CpuRiver_Functional::reset(IFace *isource) {
    CpuGeneric::reset(isource);
    portRegs_.reset();
    uint64_t misa = portCSR_.read(CSR_misa).val;
    portCSR_.reset();
    portCSR_.write(CSR_mvendorid, vendorid_.to_uint64());
    portCSR_.write(CSR_mimplementationid, implementationid_.to_uint64());
    portCSR_.write(CSR_mhartid, hartid_.to_uint64());
    portCSR_.write(CSR_mtvec, 0);
    portCSR_.write(CSR_misa, misa);

    cur_prv_level = PRV_M;           // Current privilege level
    mmuReservedAddrWatchdog_ = 0;
}

GenericInstruction *CpuRiver_Functional::decodeInstruction(Reg64Type *cache) {
    RiscvInstruction *instr = NULL;
    int hash_idx = hash32(cacheline_[0].buf32[0]);
    for (unsigned i = 0; i < listInstr_[hash_idx].size(); i++) {
        instr = static_cast<RiscvInstruction *>(
                        listInstr_[hash_idx][i].to_iface());
        if (instr->parse(cacheline_[0].buf32)) {
            break;
        }
        instr = NULL;
    }
    // Check compressed instructions:
    if (instr == NULL) {
        hash_idx = hash16(cacheline_[0].buf16[0]);
        for (unsigned i = 0; i < listInstr_[hash_idx].size(); i++) {
            instr = static_cast<RiscvInstruction *>(
                            listInstr_[hash_idx][i].to_iface());
            if (instr->parse(cacheline_[0].buf32)) {
                break;
            }
            instr = NULL;
        }
    }
    if (mmuReservedAddrWatchdog_) {
        mmuReservedAddrWatchdog_--;
    }
    return instr;
}

void CpuRiver_Functional::generateIllegalOpcode() {
    raiseSignal(EXCEPTION_InstrIllegal);
    RISCV_error("Illegal instruction at 0x%08" RV_PRI64 "x", getPC());
}

void CpuRiver_Functional::trackContextStart() {
    CpuGeneric::trackContextStart();
    if (trace_file_ == 0) {
        return;
    }
}

void CpuRiver_Functional::traceOutput() {
    char tstr[1024];

    riscv_disassembler(trace_data_.instr,
                       trace_data_.disasm,
                       sizeof(trace_data_.disasm));

    RISCV_sprintf(tstr, sizeof(tstr),
        "%9" RV_PRI64 "d: %08" RV_PRI64 "x: %s \r\n",
            trace_data_.step_cnt,
            trace_data_.pc,
            trace_data_.disasm);
    (*trace_file_) << tstr;


    for (int i = 0; i < trace_data_.action_cnt; i++) {
        trace_action_type *pa = &trace_data_.action[i];
        if (!pa->memop) {
            RISCV_sprintf(tstr, sizeof(tstr),
                "%20s %10s <= %016" RV_PRI64 "x\r\n",
                    "",
                    IREGS_NAMES[pa->waddr],
                    pa->wdata);
        } else if (pa->memop_write) {
            RISCV_sprintf(tstr, sizeof(tstr),
                "%20s [%08" RV_PRI64 "x] <= %016" RV_PRI64 "x\r\n",
                    "",
                    pa->memop_addr,
                    pa->memop_data.val);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr),
                "%20s [%08" RV_PRI64 "x] => %016" RV_PRI64 "x\r\n",
                    "",
                    pa->memop_addr,
                    pa->memop_data.val);
        }
        (*trace_file_) << tstr;
    }

    trace_file_->flush();
}

void CpuRiver_Functional::raiseSignal(int idx) {
    if (idx < SIGNAL_HardReset) {
        interrupt_pending_[idx >> 6] |= 1LL << (idx & 0x3F);
    } else if (idx == SIGNAL_HardReset) {
    } else {
        RISCV_error("Raise unsupported signal %d", idx);
    }
}

void CpuRiver_Functional::lowerSignal(int idx) {
    if (idx == SIGNAL_HardReset) {
    } else if (idx < SIGNAL_HardReset) {
        interrupt_pending_[idx >> 6] &= ~(1ull << (idx & 0x3F));
    } else {
        RISCV_error("Lower unsupported signal %d", idx);
    }
}

void CpuRiver_Functional::exceptionLoadInstruction(Axi4TransactionType *tr) {
    portCSR_.write(CSR_mtval, tr->addr);
    raiseSignal(EXCEPTION_InstrFault);
}

void CpuRiver_Functional::exceptionLoadData(Axi4TransactionType *tr) {
    portCSR_.write(CSR_mtval, tr->addr);
    raiseSignal(EXCEPTION_LoadFault);
}

void CpuRiver_Functional::exceptionStoreData(Axi4TransactionType *tr) {
    portCSR_.write(CSR_mtval, tr->addr);
    raiseSignal(EXCEPTION_StoreFault);
}

}  // namespace debugger

