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
    registerAttribute("CLINT", &clint_);
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

    iirqext_ = static_cast<IIrqController *>(RISCV_get_service_iface(
        plic_.to_string(), IFACE_IRQ_CONTROLLER));
    if (!iirqext_) {
        RISCV_error("Interface IIrqController in %s not found",
                    plic_.to_string());
    }

    iirqloc_ = static_cast<IIrqController *>(RISCV_get_service_iface(
        clint_.to_string(), IFACE_IRQ_CONTROLLER));
    if (!iirqloc_) {
        RISCV_error("Interface IIrqController in %s not found",
                    clint_.to_string());
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

/** Check stack protection exceptions: */
void CpuRiver_Functional::checkStackProtection() {
    uint64_t mstackovr = readCSR(CSR_mstackovr);
    uint64_t mstackund = readCSR(CSR_mstackund);
    uint64_t sp = portRegs_.read(Reg_sp).val;
    if (mstackovr != 0 && sp < mstackovr) {
        generateException(EXCEPTION_StackOverflow, getPC());
        writeCSR(CSR_mstackovr, 0);
    } else if (mstackund != 0 && sp > mstackund) {
        generateException(EXCEPTION_StackUnderflow, getPC());
        writeCSR(CSR_mstackund, 0);
    }
}

void CpuRiver_Functional::handleException(int e) {
    if (e == EXCEPTION_Breakpoint && estate_ == CORE_ProgbufExec) {
        exitProgbufExec();
        return;
    }

    if (estate_ != CORE_ProgbufExec) {
        csr_mcause_type mcause;
        mcause.bits.irq = 0;
        mcause.bits.code = e;
        writeCSR(CSR_mcause, mcause.value);
    }

    DCSR_TYPE::ValueType dcsr;
    dcsr.val = static_cast<uint32_t>(readCSR(CSR_dcsr));
    if (e == EXCEPTION_Breakpoint && dcsr.bits.ebreakm == 1) {
        setNPC(getPC());
        halt(HALT_CAUSE_EBREAK, "EBREAK Breakpoint");
        return;
    }

    switchContext(PRV_M);

    uint64_t mtvec = readCSR(CSR_mtvec) & ~0x3ull;
    setNPC(mtvec);
}

void CpuRiver_Functional::handleInterrupts() {
    int ctx = 0;
    csr_mcause_type mcause;
    csr_mstatus_type mstatus;
    mstatus.value = readCSR(CSR_mstatus);
    if (mstatus.bits.MIE == 0) {
        return;
    }

    csr_mie_type mie;
    mie.value = readCSR(CSR_mie);

    // Check software interrupt
    mcause.value = 0;
    if (mie.bits.MSIE == 1) {
        if (iirqloc_->getPendingRequest(2*hartid_.to_int())) {
            mcause.bits.irq = 1;
            mcause.bits.code = 3;
        }
    }

    // Check mtimer interrupt
    if (!mcause.bits.irq && mie.bits.MTIE == 1) {
        if (iirqloc_->getPendingRequest(2*hartid_.to_int() + 1)) {
            mcause.bits.irq = 1;
            mcause.bits.code = 7;
        }
    }

    // Check PLIC interrupt request
    if (!mcause.bits.irq && mie.bits.MEIE == 1) {
        // external interrupt disabled
        int irqidx = iirqext_->getPendingRequest(ctx);
        if (irqidx != IRQ_REQUEST_NONE) {
            mcause.bits.irq = 1;
            mcause.bits.code = 11;
        }
    }

    if (mcause.bits.irq) {
        writeCSR(CSR_mcause, mcause.value);

        switchContext(PRV_M);

        uint64_t mtvec = readCSR(CSR_mtvec);
        uint64_t mtvecmode = mtvec & 0x3;
        mtvec &= ~0x3ull;
        // Vector table only for interrupts (not for exceptions):
        if (mtvecmode == 0x1) {
            setNPC(mtvec + 4 * mcause.bits.code);
        } else {
            setNPC(mtvec);
        }
    }
}

void CpuRiver_Functional::switchContext(uint32_t prvnxt) {
    // All traps handle via machine mode while CSR mdelegate
    // doesn't setup other.
    // @todo delegating
    csr_mstatus_type mstatus;
    mstatus.value = readCSR(CSR_mstatus);
    mstatus.bits.MPP = cur_prv_level;
    mstatus.bits.MPIE = (mstatus.value >> cur_prv_level) & 0x1;
    mstatus.bits.MIE = 0;
    cur_prv_level = prvnxt;
    writeCSR(CSR_mstatus, mstatus.value);

    int xepc = static_cast<int>((cur_prv_level << 8) + 0x41);
    writeCSR(xepc, getNPC());
}

void CpuRiver_Functional::reset(IFace *isource) {
    CpuGeneric::reset(isource);
    portRegs_.reset();
    uint64_t misa = readCSR(CSR_misa);
    portCSR_.reset();
    writeCSR(CSR_mvendorid, vendorid_.to_uint64());
    writeCSR(CSR_mimplementationid, implementationid_.to_uint64());
    writeCSR(CSR_mhartid, hartid_.to_uint64());
    writeCSR(CSR_mtvec, 0);
    writeCSR(CSR_misa, misa);
    writeCSR(CSR_dpc, getResetAddress());

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
    generateException(EXCEPTION_InstrIllegal, getPC());
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
                    RISCV_IREGS_NAMES[pa->waddr],
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

bool CpuRiver_Functional::isStepEnabled() {
    DCSR_TYPE::ValueType dcsr;
    dcsr.val = static_cast<uint32_t>(readCSR(CSR_dcsr));
    return dcsr.bits.step;
}

void CpuRiver_Functional::enterDebugMode(uint64_t v, uint32_t cause) {
    DCSR_TYPE::ValueType dcsr;
    dcsr.val = static_cast<uint32_t>(readCSR(CSR_dcsr));
    dcsr.bits.cause = cause;
    writeCSR(CSR_dcsr, dcsr.val);
    writeCSR(CSR_dpc, v);
}


uint64_t CpuRiver_Functional::readRegDbg(uint32_t regno) {
    uint64_t rdata = 0;
    uint32_t region = regno >> 12;
    if (region == 0) {
        rdata = readCSR(regno);
    } else if (region == 1) {
        rdata = readGPR(regno & 0x3F);
    } else if (region == 0xc) {
        rdata = readNonStandardReg(regno & 0xFFF);
    }
    return rdata;
}

void CpuRiver_Functional::writeRegDbg(uint32_t regno, uint64_t val) {
    uint32_t region = regno >> 12;
    if (region == 0) {
        writeCSR(regno, val);
    } else if (region == 1) {
        writeGPR(regno & 0x3F, val);
    } else if (region == 0xc) {
        writeNonStandardReg(regno & 0xFFF, val);
    }
}

uint64_t CpuRiver_Functional::readCSR(uint32_t regno) {
    uint64_t ret = 0;
    uint64_t trigidx;
    bool rd_access = true;;
    switch (regno) {
    case CSR_mcycle:
    case CSR_minsret:
    case CSR_cycle:
    case CSR_time:
    case CSR_insret:
        ret = step_cnt_;
        rd_access = false;
        break;
    case CSR_dpc:
        if (!isHalted()) {
            ret = getNPC();
            rd_access = false;
        }
        break;
    case CSR_tdata1:
        trigidx = readCSR(CSR_tselect);
        ret = ptriggers_[trigidx].data1.val;
        rd_access = false;
        break;
    case CSR_tdata2:
        trigidx = readCSR(CSR_tselect);
        ret = ptriggers_[trigidx].data2;
        rd_access = false;
        break;
    case CSR_textra:
        trigidx = readCSR(CSR_tselect);
        ret = ptriggers_[trigidx].extra;
        rd_access = false;
        break;
    case CSR_tinfo:
        // RO: list of supported triggers
        ret = (1ull << TriggerType_AddrDataMatch)
            | (1ull << TriggerType_InstrCountMatch)
            | (1ull << TriggerType_Inetrrupt)
            | (1ull << TriggerType_Exception);
        rd_access = false;
        break;
    case CSR_mip:
        {
            int hartid = hartid_.to_int();
            csr_mip_type mip;
            mip.value = 0;
            mip.bits.MSIP = iirqloc_->getPendingRequest(2*hartid);
            mip.bits.MTIP = iirqloc_->getPendingRequest(2*hartid + 1);
            mip.bits.MEIP = iirqext_->getPendingRequest(hartid) != IRQ_REQUEST_NONE;
            ret = mip.value;
            rd_access = false;
        }
        break;
    default:;
    }
    if (rd_access) {
        RISCV_mutex_lock(&mutex_csr_);
        ret = portCSR_.read(regno).val;
        RISCV_mutex_unlock(&mutex_csr_);
    }
    return ret;
}

void CpuRiver_Functional::writeCSR(uint32_t regno, uint64_t val) {
    bool wr_access = true;
    uint64_t trigidx;
    switch (regno) {
    // Read-Only registers
    case CSR_misa:
    case CSR_mvendorid:
    case CSR_marchid:
    case CSR_mimplementationid:
    case CSR_mhartid:
        wr_access = false;
        break;
    // read only timers:
    case CSR_mcycle:
    case CSR_minsret:
    case CSR_cycle:
    case CSR_time:
    case CSR_insret:
        wr_access = false;
        break;
    case CSR_tselect:
        if (val > triggersTotal_.to_uint64()) {
            val = triggersTotal_.to_uint64();
            RISCV_debug("Select trigger %d", static_cast<int>(val));
        }
        break;
    case CSR_tdata1:
        trigidx = readCSR(CSR_tselect);
        TriggerData1Type tdata1;
        tdata1.val = val;
        if (tdata1.bitsdef.type == TriggerType_AddrDataMatch) {
            // Preset value
            tdata1.mcontrol_bits.maskmax = mcontrolMaskmax_.to_uint64();
        }
        ptriggers_[trigidx].data1.val = val;
        RISCV_info("[tdata1] <= %016" RV_PRI64 "x, type=%d",
            val, static_cast<uint32_t>(tdata1.bitsdef.type));
        val = tdata1.val;
        break;
    case CSR_tdata2:
        trigidx = readCSR(CSR_tselect);
        ptriggers_[trigidx].data2 = val;
        RISCV_info("[tdata2] <= %016" RV_PRI64 "x", val);
        break;
    case CSR_textra:
        trigidx = readCSR(CSR_tselect);
        ptriggers_[trigidx].extra = val;
        RISCV_info("[textra] <= %016" RV_PRI64 "x", val);
        break;
    case CSR_flushi:
        flush(val);
        break;
    case CSR_satp:
        if ((val >> 60) & 0xf) {
            RISCV_error(
                "[satp] <= %016" RV_PRI64 "x. Paging not supported", val);
        }
        break;
    default:;
    }
    if (wr_access) {
        RISCV_mutex_lock(&mutex_csr_);
        portCSR_.write(regno, val);
        RISCV_mutex_unlock(&mutex_csr_);
    }
}


}  // namespace debugger

