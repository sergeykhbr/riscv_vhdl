/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU functional simulator class definition.
 */

#include "api_core.h"
#include "cpu_riscv_func.h"
#include "riscv-isa.h"
#if 1
#include "coreservices/iserial.h"
#endif

namespace debugger {

#ifdef ENABLE_OBOSOLETE
void addIsaUserRV64I(CpuContextType *data, AttributeType *out);
void addIsaPrivilegedRV64I(CpuContextType *data, AttributeType *out);
void addIsaExtensionA(CpuContextType *data, AttributeType *out);
void addIsaExtensionF(CpuContextType *data, AttributeType *out);
void addIsaExtensionM(CpuContextType *data, AttributeType *out);

void generateException(uint64_t code, CpuContextType *data);
void generateInterrupt(uint64_t code, CpuContextType *data);

uint64_t readCSR(uint32_t idx, CpuContextType *data);
void writeCSR(uint32_t idx, uint64_t val, CpuContextType *data);

CpuRiscV_Functional::CpuRiscV_Functional(const char *name)  
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<ICpuGeneric *>(this));
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("Bus", &bus_);
    registerAttribute("ListExtISA", &listExtISA_);
    registerAttribute("FreqHz", &freqHz_);
    registerAttribute("GenerateRegTraceFile", &generateRegTraceFile_);
    registerAttribute("GenerateMemTraceFile", &generateMemTraceFile_);
    registerAttribute("ResetVector", &resetVector_);

    isEnable_.make_boolean(true);
    bus_.make_string("");
    listExtISA_.make_list(0);
    freqHz_.make_uint64(1);
    generateRegTraceFile_.make_boolean(false);
    generateMemTraceFile_.make_boolean(false);

    cpu_context_.step_cnt = 0;
    cpu_context_.stack_trace_cnt = 0;

    RISCV_event_create(&config_done_, "riscv_func_config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
    cpu_context_.reset   = true;
    dbg_state_ = STATE_Normal;
    last_hit_breakpoint_ = ~0;

    trans_.source_idx = CFG_NASTI_MASTER_CACHED;
    cpu_context_.reg_trace_file = 0;
    cpu_context_.mem_trace_file = 0;
    dport.valid = 0;
}
#endif

CpuRiver_Functional::CpuRiver_Functional(const char *name) :
    CpuGeneric(name),
    portRegs_(static_cast<IService *>(this), "regs", 1<<15, Reg_Total*8),
    portCSR_(static_cast<IService *>(this), "csr", 0x0, (1<<12)*8) {
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerAttribute("ListExtISA", &listExtISA_);
    registerAttribute("VendorID", &vendorID_);
    registerAttribute("VectorTable", &vectorTable_);
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
        } else if (listExtISA_[i].to_string()[0] == 'F') {
            addIsaExtensionF();
        } else if (listExtISA_[i].to_string()[0] == 'M') {
            addIsaExtensionM();
        }
    }

    // Power-on
    reset(false);

    CpuGeneric::postinitService();
}

unsigned CpuRiver_Functional::addSupportedInstruction(
                                    RiscvInstruction *instr) {
    AttributeType tmp(instr);
    listInstr_[instr->hash()].add_to_list(&tmp);
    return 0;
}


#ifdef ENABLE_OBOSOLETE

void CpuRiscV_Functional::updatePipeline() {
    IInstruction *instr;
    CpuContextType *pContext = getpContext();

    if (dport.valid) {
        dport.valid = 0;
        updateDebugPort();
    }

    pContext->pc = pContext->npc;
    if (isRunning()) {
        fetchInstruction();
    }

    updateState();
    if (pContext->reset) {
        updateQueue();
        reset();
        return;
    } 

    instr = decodeInstruction(cacheline_);
    if (isRunning()) {
        last_hit_breakpoint_ = ~0;
        if (instr) {
            executeInstruction(instr, cacheline_);
        } else {
            pContext->npc += 4;
            generateException(EXCEPTION_InstrIllegal, pContext);

            RISCV_info("[%" RV_PRI64 "d] pc:%08x: %08x \t illegal instruction",
                        getStepCounter(),
                        static_cast<uint32_t>(pContext->pc), cacheline_[0]);
        }
    }

    updateQueue();

    handleTrap();
}

void CpuRiscV_Functional::updateState() {
    CpuContextType *pContext = getpContext();
    bool upd = true;
    switch (dbg_state_) {
    case STATE_Halted:
        upd = false;
        break;
    case STATE_Stepping:
        if (dbg_step_cnt_ <= pContext->step_cnt) {
            halt("Stepping breakpoint");
            upd = false;
        }
        break;
    default:;
    }
    if (upd) {
        pContext->step_cnt++;
   }
}
#endif

void CpuRiver_Functional::handleTrap() {
    csr_mstatus_type mstatus;
    csr_mcause_type mcause;
    if (interrupt_pending_ == 0) {
        return;
    }

    mstatus.value = portCSR_.read(CSR_mstatus).val;
    mcause.value =  portCSR_.read(CSR_mcause).val;
    uint64_t exception_mask = (1ull << INTERRUPT_USoftware) - 1;
    if ((interrupt_pending_ & exception_mask) == 0 && 
        mstatus.bits.MIE == 0 && cur_prv_level == PRV_M) {
        return;
    }
    if (mcause.value == EXCEPTION_Breakpoint
        && br_ctrl.bits.trap_on_break == 0) {
        interrupt_pending_ &= ~(1ull << EXCEPTION_Breakpoint);
        npc_.setValue(pc_.getValue());
        halt("EBREAK Breakpoint");
        return;
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
    if (interrupt_pending_ & exception_mask) {
        // Exception
        portCSR_.write(xepc, pc_.getValue().val);
    } else {
        // Software interrupt handled after instruction was executed
        portCSR_.write(xepc,  npc_.getValue().val);
    }
    npc_.setValue(portCSR_.read(CSR_mtvec));
    interrupt_pending_ = 0;
}

#ifdef ENABLE_OBOSOLETE
bool CpuRiscV_Functional::isRunning() {
    return  (dbg_state_ != STATE_Halted);
}

void CpuRiscV_Functional::reset() {
    CpuContextType *pContext = getpContext();
    pContext->regs[0] = 0;
    pContext->pc = resetVector_.to_uint64();
    pContext->npc = resetVector_.to_uint64();
    pContext->exception = 0;
    pContext->interrupt = 0;
    pContext->interrupt_pending = 0;
    pContext->csr[CSR_mvendorid] = 0x0001;   // UC Berkeley Rocket repo
    pContext->csr[CSR_mhartid] = 0;
    pContext->csr[CSR_marchid] = 0;
    pContext->csr[CSR_mimplementationid] = 0;
    pContext->csr[CSR_mtvec]   = 0x100;     // Hardwired RO value
    pContext->csr[CSR_mip] = 0;             // clear pending interrupts
    pContext->csr[CSR_mie] = 0;             // disabling interrupts
    pContext->csr[CSR_mepc] = 0;
    pContext->csr[CSR_mcause] = 0;
    pContext->csr[CSR_medeleg] = 0;
    pContext->csr[CSR_mideleg] = 0;
    pContext->csr[CSR_mtime] = 0;
    pContext->csr[CSR_mtimecmp] = 0;
    pContext->csr[CSR_uepc] = 0;
    pContext->csr[CSR_sepc] = 0;
    pContext->csr[CSR_hepc] = 0;
    csr_mstatus_type mstat;
    mstat.value = 0;
    pContext->csr[CSR_mstatus] = mstat.value;
    pContext->cur_prv_level = PRV_M;           // Current privilege level
    pContext->step_cnt = 0;
    pContext->br_ctrl.val = 0;
    pContext->br_inject_fetch = false;
    pContext->br_status_ena = false;
    pContext->stack_trace_cnt = 0;
}
#endif

void CpuRiver_Functional::reset(bool active) {
    CpuGeneric::reset(active);
    portRegs_.reset();
    portCSR_.reset();
    portCSR_.write(CSR_mvendorid, vendorID_.to_uint64());
    portCSR_.write(CSR_mtvec, vectorTable_.to_uint64());

    cur_prv_level = PRV_M;           // Current privilege level
    br_ctrl.val = 0;
    br_inject_fetch = false;
    br_status_ena = false;
}


#ifdef ENABLE_OBOSOLETE
void CpuRiscV_Functional::fetchInstruction() {
    CpuContextType *pContext = getpContext();
    if (pContext->br_inject_fetch
        && pContext->pc == pContext->br_address_fetch) {
        pContext->br_inject_fetch = false;
        cacheline_[0] = pContext->br_instr_fetch;
        return;
    }
    trans_.action = MemAction_Read;
    trans_.addr = pContext->pc;
    trans_.xsize = 4;
    trans_.wstrb = 0;
    pContext->ibus->b_transport(&trans_);
    cacheline_[0] = trans_.rpayload.b32[0];
}

#endif

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
    return instr;
}

void CpuRiver_Functional::generateIllegalOpcode() {
    raiseSignal(EXCEPTION_InstrIllegal);
    RISCV_error("Illegal instruction at 0x%"RV_PRI64 "08x", getPC());
}

#ifdef ENABLE_OBOSOLETE
void CpuRiscV_Functional::executeInstruction(IInstruction *instr,
                                             uint32_t *rpayload) {

    CpuContextType *pContext = getpContext();
    if (pContext->reg_trace_file) {
        /** Save previous reg values to find modification after exec() */
        for (int i = 0; i < Reg_Total; i++) {
            iregs_prev[i] = pContext->regs[i];
        }
    }

    instr->exec(cacheline_, pContext);
    if (pContext->reg_trace_file) {
        int sz;
        sz = RISCV_sprintf(tstr, sizeof(tstr),"%8I64d [%08x] %08x: ",
            pContext->step_cnt, static_cast<uint32_t>(pContext->pc), rpayload[0]);

        bool reg_changed = false;
        for (int i = 0; i < 32; i++) {
            if (iregs_prev[i] != pContext->regs[i]) {
                reg_changed = true;
                sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz,
                        "%3s <= %016I64x\n", IREGS_NAMES[i], pContext->regs[i]);
            }
        }
        if (!reg_changed) {
            sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz, "%s", "-\n");
        }
        (*pContext->reg_trace_file) << tstr;
        pContext->reg_trace_file->flush();
    }
}
#endif

#ifdef ENABLE_OBOSOLETE
/** 
 * prv-1.9.1 page 29
 *
 * An interrupt i will be taken if bit i is set in both mip and mie, 
 * and if interrupts are globally enabled.  By default, M-mode interrupts 
 * are globally enabled if the hart’s current privilege mode is less than M,
 * or if the current privilege mode is M and the MIE bit in the mstatus
 * register is set.
 * If bit i in mideleg is set, however, interrupts are considered to be
 * globally enabled if the hart’s current privilege mode equals the delegated
 * privilege mode (H, S, or U) and that mode’s interrupt enable bit 
 * (HIE, SIE or UIE in mstatus) is set, or if the current privilege mode is
 * less than the delegated privilege mode.
 */
void CpuRiscV_Functional::raiseSignal(int idx) {
    CpuContextType *pContext = getpContext();
    csr_mstatus_type mstatus;
    mstatus.value = pContext->csr[CSR_mstatus];

    switch (idx) {
    case CPU_SIGNAL_RESET:
        pContext->reset = true; // Active HIGH
        break;
    case CPU_SIGNAL_EXT_IRQ:
        if (pContext->reset) {
            break;
        }
        // External Interrupt controller pending bit
        pContext->interrupt_pending |= (1ull << idx);
        if (mstatus.bits.MIE == 0 && pContext->cur_prv_level == PRV_M) {
            break;
        }
        /// @todo delegate interrupt to non-machine privilege level.
        generateInterrupt(INTERRUPT_MExternal, pContext);
        break;
    default:
        RISCV_error("Unsupported signalRaise(%d)", idx);
    }
}

void CpuRiscV_Functional::lowerSignal(int idx) {
    CpuContextType *pContext = getpContext();
    switch (idx) {
    case CPU_SIGNAL_RESET:
        pContext->reset = false; // Active HIGH
        break;
    case CPU_SIGNAL_EXT_IRQ:
        pContext->interrupt_pending &= ~(1 << idx);
        break;
    default:
        RISCV_error("Unsupported lowerSignal(%d)", idx);
    }
}
#endif

void CpuRiver_Functional::raiseSignal(int idx) {
    if (idx < INTERRUPT_USoftware) {
        // Exception:
        csr_mcause_type cause;
        cause.value     = 0;
        cause.bits.irq  = 0;
        cause.bits.code = idx;
        portCSR_.write(CSR_mcause, cause.value);
        interrupt_pending_ |= 1LL << idx;
        if (idx == EXCEPTION_Breakpoint) {
            br_status_ena = true;
        }
    } else if (idx < SIGNAL_HardReset) {
        csr_mcause_type cause;
        cause.value     = 0;
        cause.bits.irq  = 1;
        cause.bits.code = idx - INTERRUPT_USoftware;
        portCSR_.write(CSR_mcause, cause.value);
        interrupt_pending_ |= 1LL << idx;
    } else if (idx == SIGNAL_HardReset) {
    } else {
        RISCV_error("Raise unsupported signal %d", idx);
    }
}

void CpuRiver_Functional::lowerSignal(int idx) {
    if (idx == SIGNAL_HardReset) {
    } else if (idx < SIGNAL_HardReset) {
        interrupt_pending_ &= ~(1 << idx);
    } else {
        RISCV_error("Lower unsupported signal %d", idx);
    }
}

uint64_t CpuRiver_Functional::readCSR(int idx) {
    if (idx == CSR_mtime) {
        return step_cnt_;
    }
    return portCSR_.read(idx).val;
}

void CpuRiver_Functional::writeCSR(int idx, uint64_t val) {
    switch (idx) {
    // Read-Only registers
    case CSR_misa:
    case CSR_mvendorid:
    case CSR_marchid:
    case CSR_mimplementationid:
    case CSR_mhartid:
        break;
    case CSR_mtime:
        break;
    default:
        portCSR_.write(idx, val);
    }
}


#ifdef ENABLE_OBOSOLETE
void CpuRiscV_Functional::updateDebugPort() {
    CpuContextType *pContext = getpContext();
    DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
    DebugPortTransactionType *trans = dport.trans;
    trans->rdata = 0;
    switch (trans->region) {
    case 0:     // CSR
        trans->rdata = pContext->csr[trans->addr];
        if (trans->write) {
            pContext->csr[trans->addr] = trans->wdata;
        }
        break;
    case 1:     // IRegs
        if (trans->addr < Reg_Total) { 
            trans->rdata = pContext->regs[trans->addr];
            if (trans->write) {
                pContext->regs[trans->addr] = trans->wdata;
            }
        } else if (trans->addr == Reg_Total) {
            /** Read only register */
            trans->rdata = pContext->pc;
        } else if (trans->addr == (Reg_Total + 1)) {
            trans->rdata = pContext->npc;
            if (trans->write) {
                pContext->npc = trans->wdata;
            }
        } else if (trans->addr == (Reg_Total + 2)) {
            trans->rdata = pContext->stack_trace_cnt;
            if (trans->write) {
                pContext->stack_trace_cnt = static_cast<int>(trans->wdata);
            }
        } else if (trans->addr >= 128 && 
                    trans->addr < (128 + STACK_TRACE_BUF_SIZE)) {
            trans->rdata = pContext->stack_trace_buf[trans->addr - 128];
        }
        break;
    case 2:     // Control
        switch (trans->addr) {
        case 0:
            ctrl.val = trans->wdata;
            if (trans->write) {
                if (ctrl.bits.halt) {
                    halt();
                } else if (ctrl.bits.stepping) {
                    step(dport.stepping_mode_steps);
                } else {
                    go();
                }
            } else {
                ctrl.val = 0;
                ctrl.bits.halt = isHalt() ? 1: 0;
                if (pContext->br_status_ena) {
                    ctrl.bits.breakpoint = 1;
                }
                ctrl.bits.core_id = 0;
            }
            trans->rdata = ctrl.val;
            break;
        case 1:
            trans->rdata = dport.stepping_mode_steps;
            if (trans->write) {
                dport.stepping_mode_steps = trans->wdata;
            }
            break;
        case 2:
            trans->rdata = pContext->step_cnt;
            break;
        case 3:
            trans->rdata = pContext->step_cnt;
            break;
        case 4:
            trans->rdata = pContext->br_ctrl.val;
            if (trans->write) {
                pContext->br_ctrl.val = trans->wdata;
            }
            break;
        case 5:
            if (trans->write) {
                addBreakpoint(trans->wdata);
            }
            break;
        case 6:
            if (trans->write) {
                removeBreakpoint(trans->wdata);
            }
            break;
        case 7:
            trans->rdata = pContext->br_address_fetch;
            if (trans->write) {
                pContext->br_address_fetch = trans->wdata;
            }
            break;
        case 8:
            trans->rdata = pContext->br_instr_fetch;
            if (trans->write) {
                pContext->br_inject_fetch = true;
                pContext->br_status_ena = false;
                pContext->br_instr_fetch = static_cast<uint32_t>(trans->wdata);
            }
            break;
        default:;
        }
        break;
    default:;
    }
    dport.cb->nb_response_debug_port(dport.trans);
}


void CpuRiscV_Functional::addBreakpoint(uint64_t addr) {
    //CpuContextType *pContext = getpContext();
    RISCV_error("Hardware breakpoints not implemented", 0);
}

void CpuRiscV_Functional::removeBreakpoint(uint64_t addr) {
    //CpuContextType *pContext = getpContext();
    RISCV_error("Hardware breakpoints not implemented", 0);
}

void CpuRiscV_Functional::hitBreakpoint(uint64_t addr) {
    CpuContextType *pContext = getpContext();
    if (addr == last_hit_breakpoint_) {
        return;
    }
    dbg_state_ = STATE_Halted;
    last_hit_breakpoint_ = addr;

    RISCV_printf0("[%" RV_PRI64 "d] pc:%016" RV_PRI64 "x: %08x \t stop on breakpoint",
        getStepCounter(), pContext->pc, cacheline_[0]);
}
#endif

}  // namespace debugger

