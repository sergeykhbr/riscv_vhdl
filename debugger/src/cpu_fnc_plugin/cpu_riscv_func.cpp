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

CpuRiscV_Functional::CpuRiscV_Functional(const char *name)  
    : IService(name), IHap(HAP_ConfigDone) {

    cpu_context_.reset   = true;
    dbg_state_ = STATE_Normal;
    last_hit_breakpoint_ = ~0;
}
#endif

CpuRiver_Functional::CpuRiver_Functional(const char *name) :
    CpuGeneric(name),
    portRegs_(this, "regs", 1<<15, Reg_Total*8),
    portSavedRegs_(this, "savedregs", 0, Reg_Total*8),  // not mapped !!!
    portCSR_(this, "csr", 0x0, (1<<12)*8) {
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
    if (mcause.value == EXCEPTION_Breakpoint) {
        DsuMapType::udbg_type::debug_region_type::breakpoint_control_reg t1;
        t1.val = br_control_.getValue().val;
        if (t1.bits.trap_on_break == 0) {
            sw_breakpoint_ = true;
            interrupt_pending_ &= ~(1ull << EXCEPTION_Breakpoint);
            npc_.setValue(pc_.getValue());
            halt("EBREAK Breakpoint");
            return;
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
void CpuRiscV_Functional::reset() {
    CpuContextType *pContext = getpContext();
    pContext->br_ctrl.val = 0;
    pContext->br_inject_fetch = false;
    pContext->br_status_ena = false;
}
#endif

void CpuRiver_Functional::reset(bool active) {
    CpuGeneric::reset(active);
    portRegs_.reset();
    portCSR_.reset();
    portCSR_.write(CSR_mvendorid, vendorID_.to_uint64());
    portCSR_.write(CSR_mtvec, vectorTable_.to_uint64());

    cur_prv_level = PRV_M;           // Current privilege level
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
    RISCV_error("Illegal instruction at 0x%" RV_PRI64 "08x", getPC());
}

void CpuRiver_Functional::trackContextStart() {
    if (reg_trace_file == 0) {
        return;
    }
    /** Save previous reg values to find modification after exec() */
    uint64_t *dst = portSavedRegs_.getpR64();
    uint64_t *src = portRegs_.getpR64();
    memcpy(dst, src, Reg_Total*sizeof(uint64_t));
}

void CpuRiver_Functional::trackContextEnd() {
    if (reg_trace_file == 0) {
        return;
    }
    int sz;
    char tstr[1024];
    sz = RISCV_sprintf(tstr, sizeof(tstr),"%8I64d [%08x] %08x: ",
        step_cnt_, pc_.getValue().buf32[0], cacheline_[0].buf32[0]);  

    bool reg_changed = false;
    uint64_t *prev = portSavedRegs_.getpR64();
    uint64_t *cur = portRegs_.getpR64();
    for (int i = 0; i < Reg_Total; i++) {
        if (prev[i] != cur[i]) {
            reg_changed = true;
            sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz,
                    "%3s <= %016I64x\n", IREGS_NAMES[i], cur[i]);
        }
    }
    if (!reg_changed) {
        sz += RISCV_sprintf(&tstr[sz], sizeof(tstr) - sz, "%s", "-\n");
    }
    (*reg_trace_file) << tstr;
    reg_trace_file->flush();
}

void CpuRiver_Functional::raiseSignal(int idx) {
    if (idx < INTERRUPT_USoftware) {
        // Exception:
        csr_mcause_type cause;
        cause.value     = 0;
        cause.bits.irq  = 0;
        cause.bits.code = idx;
        portCSR_.write(CSR_mcause, cause.value);
        interrupt_pending_ |= 1LL << idx;
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

