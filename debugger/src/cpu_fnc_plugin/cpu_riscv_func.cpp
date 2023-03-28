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
    registerAttribute("PmpTotal", &pmpTotal_);

    mmuReservatedAddr_ = 0;
    mmuReservedAddrWatchdog_ = 0;
    memset(&pmpTable_, 0, sizeof(pmpTable_));
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
}

void CpuRiver_Functional::predeleteService() {
    CpuGeneric::predeleteService();
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

    csr_dcsr_type dcsr;
    dcsr.u64 = static_cast<uint32_t>(readCSR(CSR_dcsr));
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
    //uint64_t misa = readCSR(CSR_misa);
    //portCSR_.reset();
    // TODO: reset each register separetly!!!
    writeCSR(CSR_mvendorid, vendorid_.to_uint64());
    writeCSR(CSR_mimplementationid, implementationid_.to_uint64());
    writeCSR(CSR_mhartid, hartid_.to_uint64());
    writeCSR(CSR_mtvec, 0);
    //writeCSR(CSR_misa, misa);
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

    isrc_->disasm(0,
                  trace_data_.pc,
                  &trace_data_.instrbuf,
                  &trace_data_.asmlist);

    RISCV_sprintf(tstr, sizeof(tstr),
        "%9" RV_PRI64 "d: %08" RV_PRI64 "x: %s \r\n",
            trace_data_.step_cnt,
            trace_data_.pc,
            trace_data_.asmlist[0u].to_string());
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
    csr_dcsr_type dcsr;
    dcsr.u64 = static_cast<uint32_t>(readCSR(CSR_dcsr));
    return dcsr.bits.step;
}

void CpuRiver_Functional::enterDebugMode(uint64_t v, uint32_t cause) {
    csr_dcsr_type dcsr;
    dcsr.u64 = static_cast<uint32_t>(readCSR(CSR_dcsr));
    dcsr.bits.cause = cause;
    writeCSR(CSR_dcsr, dcsr.u64);
    writeCSR(CSR_dpc, v);
}


int CpuRiver_Functional::dportReadReg(uint32_t regno, uint64_t *val) {
    *val = 0;
    uint32_t region = regno >> 12;
    if (region == 0) {
        *val = readCSR(regno);
    } else if (region == 1) {
        *val = readGPR(regno & 0x3F);
    } else if (region == 0xc) {
        *val = readNonStandardReg(regno & 0xFFF);
    } else {
        return -1;
    }
    return 0;
}

int CpuRiver_Functional::dportWriteReg(uint32_t regno, uint64_t val) {
    uint32_t region = regno >> 12;
    if (region == 0) {
        writeCSR(regno, val);
    } else if (region == 1) {
        writeGPR(regno & 0x3F, val);
    } else if (region == 0xc) {
        writeNonStandardReg(regno & 0xFFF, val);
    } else {
        return -1;
    }
    return 0;
}

int CpuRiver_Functional::dportReadMem(uint64_t addr, uint32_t virt,
                                      uint32_t sz, uint64_t *payload) {
    Axi4TransactionType tr;
    tr.action = MemAction_Read;
    tr.source_idx = sysBusMasterID_.to_int();
    tr.addr = addr;
    //if (virt) {
    //    tr.addr = va2pa(addr);
    //}
    tr.xsize = sz;
    if (dma_memop(&tr) != TRANS_OK) {
        return -1;
    }
    memcpy(payload, tr.rpayload.b8, sz);
    return 0;
}

int CpuRiver_Functional::dportWriteMem(uint64_t addr, uint32_t virt,
                                       uint32_t sz, uint64_t payload) {
    Axi4TransactionType tr;
    tr.action = MemAction_Write;
    tr.source_idx = sysBusMasterID_.to_int();
    tr.addr = addr;
    //if (virt) {
    //    tr.addr = va2pa(addr);
    //}
    tr.xsize = sz;
    tr.wstrb = (1 << sz) - 1;
    tr.wpayload.b64[0] = payload;
    if (dma_memop(&tr) != TRANS_OK) {
        return -1;
    }
    return 0;
}


uint64_t CpuRiver_Functional::readCSR(uint32_t regno) {
    uint64_t ret = 0;
    uint64_t trigidx;
    if (regno == CSR_mcycle) {
        ret = step_cnt_;
    } else if (regno == CSR_minsret) {
        ret = step_cnt_;
    } else if (regno == CSR_cycle) {
        ret = step_cnt_;
    } else if (regno == CSR_time) {
        ret = step_cnt_;
    } else if (regno == CSR_insret) {
        ret = step_cnt_;
    } else if (regno == CSR_dpc) {
        if (!isHalted()) {
            ret = getNPC();
        } else {
            ret = portCSR_.read(regno).val;
        }
    } else if (regno == CSR_tdata1) {
        trigidx = readCSR(CSR_tselect);
        ret = ptriggers_[trigidx].data1.val;
    } else if (regno == CSR_tdata2) {
        trigidx = readCSR(CSR_tselect);
        ret = ptriggers_[trigidx].data2;
    } else if (regno == CSR_textra) {
        trigidx = readCSR(CSR_tselect);
        ret = ptriggers_[trigidx].extra;
    } else if (regno == CSR_tinfo) {
        // RO: list of supported triggers
        ret = (1ull << TriggerType_AddrDataMatch)
            | (1ull << TriggerType_InstrCountMatch)
            | (1ull << TriggerType_Inetrrupt)
            | (1ull << TriggerType_Exception);
    } else if (regno == CSR_mip) {
        int hartid = hartid_.to_int();
        csr_mip_type mip;
        mip.value = 0;
        mip.bits.MSIP = iirqloc_->getPendingRequest(2*hartid);
        mip.bits.MTIP = iirqloc_->getPendingRequest(2*hartid + 1);
        mip.bits.MEIP = iirqext_->getPendingRequest(hartid) != IRQ_REQUEST_NONE;
        ret = mip.value;
    } else {
        RISCV_mutex_lock(&mutex_csr_);
        ret = portCSR_.read(regno).val;
        RISCV_mutex_unlock(&mutex_csr_);
    }
    return ret;
}

void CpuRiver_Functional::writeCSR(uint32_t regno, uint64_t val) {
    bool wr_access = true;
    uint64_t trigidx;
    // Read-Only registers
    if (regno == CSR_misa) {
        wr_access = false;  // RO
    } else if (regno == CSR_mvendorid) {
        wr_access = false;  // RO
    } else if (regno == CSR_marchid) {
        wr_access = false;  // RO
    } else if (regno == CSR_mimplementationid) {
        wr_access = false;  // RO
    } else if (regno == CSR_mhartid) {
        wr_access = false;  // RO
    } else if (regno == CSR_mcycle) {
        wr_access = false;  // RO
    } else if (regno == CSR_minsret) {
        wr_access = false;  // RO
    } else if (regno == CSR_cycle) {
        wr_access = false;  // RO
    } else if (regno == CSR_time) {
        wr_access = false;  // RO
    } else if (regno == CSR_insret) {
        wr_access = false;  // RO
    } else if (regno == CSR_tselect) {
        if (val > triggersTotal_.to_uint64()) {
            val = triggersTotal_.to_uint64();
            RISCV_debug("Select trigger %d", static_cast<int>(val));
        }
    } else if (regno == CSR_tdata1) {
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
    } else if (regno == CSR_tdata2) {
        trigidx = readCSR(CSR_tselect);
        ptriggers_[trigidx].data2 = val;
        RISCV_info("[tdata2] <= %016" RV_PRI64 "x", val);
    } else if (regno == CSR_textra) {
        trigidx = readCSR(CSR_tselect);
        ptriggers_[trigidx].extra = val;
        RISCV_info("[textra] <= %016" RV_PRI64 "x", val);
    } else if (regno == CSR_flushi) {
        flush(val);
    } else if (regno >= CSR_pmpcfg0 && regno <= CSR_pmpcfg15) {
        // Physical memory protection configuration:
        uint64_t mask54 = (1ull << 54) - 1;
        unsigned pmpidx = 8 * (regno - (CSR_pmpcfg0 & ~0x1));
        unsigned pmptot = 8;
        unsigned pmpcfg;
        unsigned A, RWX, L;
        if (CSR_pmpcfg0 & 0x1) {
            pmptot = 4;
            pmpidx += 4;
        }
        for (unsigned i = 0; i < pmptot; i++) {
            pmpcfg = static_cast<unsigned>((val >> (8 * i)) & 0xFF);
            RWX = pmpcfg & 0x7;
            A = (pmpcfg >> 3) & 0x3;
            L = (pmpcfg >> 7) & 0x1;
            uint64_t startaddr = 0;
            uint64_t endaddr = (mask54 << 2) | 0x3;
            if (A == 0x0) {
                disablePmp(pmpidx + i);
            } else if (A == 1) {
                // TOR: Top of region
                endaddr = (portCSR_.read(CSR_pmpaddr0 + pmpidx + i).val & mask54) - 1;
                if (pmpidx) {
                    startaddr = portCSR_.read(CSR_pmpaddr0 + pmpidx + i - 1).val & mask54;
                }
                enablePmp(pmpidx + i, startaddr, endaddr, RWX, L);
            } else if (A == 2) {
                startaddr = (portCSR_.read(CSR_pmpaddr0 + pmpidx + i).val & mask54) << 2;
                endaddr = startaddr + 3;
                enablePmp(pmpidx + i, startaddr, endaddr, RWX, L);
            } else if (A == 3) {
                startaddr = portCSR_.read(CSR_pmpaddr0 + pmpidx + i).val & mask54;
                if (startaddr == mask54) {
                    // Full memory region
                    startaddr = 0;
                    endaddr = ~0ull;
                } else {
                    uint64_t bitidx = 0x1ull;
                    while ((startaddr & bitidx) && bitidx) {
                        startaddr &= ~bitidx;
                        bitidx <<= 1;
                    }
                    startaddr <<= 2;
                    endaddr = startaddr + 8 * bitidx - 1;
                }
                enablePmp(pmpidx + i, startaddr, endaddr, RWX, L);
            }
        }
    } else if (regno == CSR_satp) {
        csr_satp_type satp;
        satp.u64 = val;
        if (satp.bits.mode != SATP_MODE_OFF
            && satp.bits.mode != SATP_MODE_SV39
            && satp.bits.mode != SATP_MODE_SV48) {
            RISCV_error(
                "[satp] <= %016" RV_PRI64 "x. Paging mode %x not supported",
                val, satp.bits.mode);
        }
    }

    if (wr_access) {
        RISCV_mutex_lock(&mutex_csr_);
        portCSR_.write(regno, val);
        RISCV_mutex_unlock(&mutex_csr_);
    }
}

void CpuRiver_Functional::disablePmp(uint32_t pmpidx) {
    pmpTable_.ena &= ~(1ull << pmpidx);
}

void CpuRiver_Functional::enablePmp(uint32_t pmpidx,
                                    uint64_t startadr,
                                    uint64_t endadr,
                                    uint32_t rwx,
                                    uint32_t lock) {
    pmpTable_.ena |= 1ull << pmpidx;
    pmpTable_.startadr[pmpidx] = startadr;
    pmpTable_.endadr[pmpidx] = endadr;

    pmpTable_.R &= ~(1ull << pmpidx);
    if (rwx & 0x1) {
        pmpTable_.R |= (1ull << pmpidx);
    }

    pmpTable_.W &= ~(1ull << pmpidx);
    if (rwx & 0x2) {
        pmpTable_.W |= (1ull << pmpidx);
    }

    pmpTable_.X &= ~(1ull << pmpidx);
    if (rwx & 0x4) {
        pmpTable_.X |= (1ull << pmpidx);
    }

    pmpTable_.L &= ~(1ull << pmpidx);
    if (lock) {
        pmpTable_.L |= (1ull << pmpidx);
    }
}

// PMP is active for S,U modes or in M-mode when L-bit is set (or MSTATUS.MPRV=1):
bool CpuRiver_Functional::isMpuEnabled() {
    uint64_t prv = getPrvLevel();
    if (prv != PRV_M) {
        return true;
    }
    csr_mstatus_type mstatus;
    mstatus.value = readCSR(CSR_mstatus);
    if (mstatus.bits.MPRV && (mstatus.bits.MPP != PRV_M)) {
        return true;
    }
    return false;
}

bool CpuRiver_Functional::checkMpu(uint64_t adr, uint32_t size, const char *rwx) {
    bool allow = false;

    for (int i = 0; i < pmpTotal_.to_int(); i++) {
        if ((pmpTable_.ena & (1ull << i)) == 0) {
            continue;
        }
        if (adr < pmpTable_.startadr[i] || adr > pmpTable_.endadr[i]) {
            continue;
        }
        if (rwx[0] == 'x' && (pmpTable_.X & (1ull << i))) {
            allow = true;
        }
        if (rwx[0] == 'r' && (pmpTable_.R & (1ull << i))) {
            allow = true;
        }
        if (rwx[0] == 'w' && (pmpTable_.W & (1ull << i))) {
            allow = true;
        }
        break;  // Lower region has higher privilege
    }
    return allow;
}

bool CpuRiver_Functional::isMmuEnabled() {
    csr_satp_type satp;
    uint64_t prv = getPrvLevel();
    satp.u64 = readCSR(CSR_satp);
    if (prv != PRV_M && satp.bits.mode) {
        return true;
    }
    csr_mstatus_type mstatus;
    mstatus.value = readCSR(CSR_mstatus);
    if (mstatus.bits.MPRV && (mstatus.bits.MPP != PRV_M) && satp.bits.mode) {
        return true;
    }
    return false;
}

uint64_t CpuRiver_Functional::translateMmu(uint64_t va) {
    uint64_t pa = va;
    if ((va >> 48) != 0xFFFF) {
        // Non-virtual address
        return pa;
    }
    return pa;
}

void CpuRiver_Functional::flushMmu() {
}

}  // namespace debugger

