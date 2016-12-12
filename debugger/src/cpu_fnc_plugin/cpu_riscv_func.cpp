/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU functional simulator class definition.
 */

#include "api_core.h"
#include "cpu_riscv_func.h"
#include "riscv-isa.h"
#include "coreservices/isocinfo.h"
#if 1
#include "coreservices/iserial.h"
#endif

namespace debugger {

void addIsaUserRV64I(CpuContextType *data, AttributeType *out);
void addIsaPrivilegedRV64I(CpuContextType *data, AttributeType *out);
void addIsaExtensionA(CpuContextType *data, AttributeType *out);
void addIsaExtensionF(CpuContextType *data, AttributeType *out);
void addIsaExtensionM(CpuContextType *data, AttributeType *out);


void generateException(uint64_t code, CpuContextType *data);

uint64_t readCSR(uint32_t idx, CpuContextType *data);
void writeCSR(uint32_t idx, uint64_t val, CpuContextType *data);


CpuRiscV_Functional::CpuRiscV_Functional(const char *name)  
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerInterface(static_cast<IClock *>(this));
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
    resetVector_.make_uint64(0x1000);

    cpu_context_.step_cnt = 0;

    RISCV_event_create(&config_done_, "config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
    cpu_context_.reset   = true;
    dbg_state_ = STATE_Normal;
    last_hit_breakpoint_ = ~0;
    reset();

    cpu_context_.reg_trace_file = 0;
    cpu_context_.mem_trace_file = 0;
    dport.valid = 0;
}

CpuRiscV_Functional::~CpuRiscV_Functional() {
    RISCV_event_close(&config_done_);
}

void CpuRiscV_Functional::postinitService() {
    CpuContextType *pContext = getpContext();

    pContext->ibus = static_cast<IBus *>(
       RISCV_get_service_iface(bus_.to_string(), IFACE_BUS));

    if (!pContext->ibus) {
        RISCV_error("Bus interface '%s' not found", 
                    bus_.to_string());
        return;
    }

    // Supported instruction sets:
    for (int i = 0; i < INSTR_HASH_TABLE_SIZE; i++) {
        listInstr_[i].make_list(0);
    }
    addIsaUserRV64I(pContext, listInstr_);
    addIsaPrivilegedRV64I(pContext, listInstr_);
    for (unsigned i = 0; i < listExtISA_.size(); i++) {
        if (listExtISA_[i].to_string()[0] == 'A') {
            addIsaExtensionA(pContext, listInstr_);
        } else if (listExtISA_[i].to_string()[0] == 'F') {
            addIsaExtensionF(pContext, listInstr_);
        } else if (listExtISA_[i].to_string()[0] == 'M') {
            addIsaExtensionM(pContext, listInstr_);
        }
    }

    // Get global settings:
    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["SimEnable"].to_bool() && isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }

        if (generateRegTraceFile_.to_bool()) {
            pContext->reg_trace_file = new std::ofstream("river_func_regs.log");
        }
        if (generateMemTraceFile_.to_bool()) {
            pContext->mem_trace_file = new std::ofstream("river_func_mem.log");
        }
    }
}

void CpuRiscV_Functional::predeleteService() {
    CpuContextType *pContext = getpContext();
    stop();
    if (pContext->reg_trace_file) {
        pContext->reg_trace_file->close();
        delete pContext->reg_trace_file;
    }
    if (pContext->mem_trace_file) {
        pContext->mem_trace_file->close();
        delete pContext->mem_trace_file;
    }
}

void CpuRiscV_Functional::hapTriggered(IFace *isrc, EHapType type,
                                       const char *descr) {
    RISCV_event_set(&config_done_);
}

void CpuRiscV_Functional::busyLoop() {
    RISCV_event_wait(&config_done_);

    while (isEnabled()) {
        updatePipeline();
    }
}

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
            halt();
            upd = false;
        }
        break;
    default:;
    }
    if (upd) {
        pContext->step_cnt++;
   }
}

void CpuRiscV_Functional::updateQueue() {
    IFace *cb;
    CpuContextType *pContext = getpContext();

    queue_.initProc();
    queue_.pushPreQueued();
        
    while ((cb = queue_.getNext(pContext->step_cnt)) != 0) {
        static_cast<IClockListener *>(cb)->stepCallback(pContext->step_cnt);
    }
}

void CpuRiscV_Functional::handleTrap() {
    CpuContextType *pContext = getpContext();
    csr_mstatus_type mstatus;
    mstatus.value = pContext->csr[CSR_mstatus];

    if (pContext->exception == 0 && pContext->interrupt == 0) {
        return;
    }
    if (pContext->interrupt == 1 && 
        mstatus.bits.MIE == 0 && pContext->cur_prv_level == PRV_M) {
        return;
    }
    pContext->interrupt = 0;
    pContext->exception = 0;

    // All traps handle via machine mode while CSR mdelegate
    // doesn't setup other.
    // @todo delegating
    mstatus.bits.MPP = pContext->cur_prv_level;
    mstatus.bits.MPIE = (mstatus.value >> pContext->cur_prv_level) & 0x1;
    mstatus.bits.MIE = 0;
    pContext->cur_prv_level = PRV_M;
    pContext->csr[CSR_mstatus] = mstatus.value;

    uint64_t xepc = (pContext->cur_prv_level << 8) + 0x41;
    if (pContext->exception) {
        pContext->csr[xepc]    = pContext->pc;
    } else {
        // Software interrupt handled after instruction was executed
        pContext->csr[xepc]    = pContext->npc;
    }
    pContext->npc = pContext->csr[CSR_mtvec];

    pContext->exception = 0;
}

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
}

void CpuRiscV_Functional::fetchInstruction() {
    CpuContextType *pContext = getpContext();
    trans_.action = MemAction_Read;
    trans_.addr = pContext->pc;
    trans_.xsize = 4;
    trans_.wstrb = 0;
    pContext->ibus->b_transport(&trans_);
    cacheline_[0] = trans_.rpayload.b32[0];
}

IInstruction *CpuRiscV_Functional::decodeInstruction(uint32_t *rpayload) {
    IInstruction *instr = NULL;
    int hash_idx = hash32(rpayload[0]);
    for (unsigned i = 0; i < listInstr_[hash_idx].size(); i++) {
        instr = static_cast<IInstruction *>(
                        listInstr_[hash_idx][i].to_iface());
        if (instr->parse(rpayload)) {
            break;
        }
        instr = NULL;
    }

    return instr;
}

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
#if 0
    //if (pContext->pc >= 0x10000000) {
    //if ((pContext->pc >= 0x100000b4 && pContext->pc <= 0x10000130)
    //|| (pContext->pc >= 0x10001ef4)
    //) 
    {
    //if (pContext->pc >= 0x10001928 && pContext->pc <= 0x10001960) {
        RISCV_debug("[%" RV_PRI64 "d] %08x: %08x \t %4s <prv=%d; mstatus=%016" RV_PRI64 "x; mcause=%016" RV_PRI64 "x; ra=%016" RV_PRI64 "x; sp=%016" RV_PRI64 "x; tp=%016" RV_PRI64 "x>", 
            getStepCounter(),
            static_cast<uint32_t>(pContext->pc),
            rpayload[0], instr->name(),
            pContext->cur_prv_level,
            pContext->csr[CSR_mstatus],
            pContext->csr[CSR_mcause],
            pContext->regs[Reg_ra],
            pContext->regs[Reg_sp],
            pContext->regs[Reg_tp]
            );
    }
#endif
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

    if (generateRegTraceFile_.to_bool()) {
        char msg[16];
        int msg_len = 0;
        IService *uart = NULL;
        switch (pContext->step_cnt) {
        case 6000:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'h';
            msg[1] = 'i';
            msg_len = 2;
            break;
        case 6500:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'g';
            msg[1] = 'h';
            msg[2] = 't';
            msg_len = 3;
            break;
        case 8200:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'i';
            msg[1] = 'c';
            msg_len = 2;
            break;
        case 8300:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'k';
            msg[1] = 's';
            msg[2] = '\r';
            msg[3] = '\n';
            msg_len = 4;
            break;
        default:;
        }

        if (uart) {
            ISerial *iserial = static_cast<ISerial *>(
                        uart->getInterface(IFACE_SERIAL));
            //iserial->writeData("pnp\r\n", 6);
            //iserial->writeData("highticks\r\n", 11);
            iserial->writeData(msg, msg_len);
        }
    }


    if (pContext->regs[0] != 0) {
        RISCV_error("Register x0 was modificated (not equal to zero)", NULL);
    }
}

void CpuRiscV_Functional::registerStepCallback(IClockListener *cb,
                                               uint64_t t) {
    if (!isEnabled()) {
        if (t <= getpContext()->step_cnt) {
            cb->stepCallback(t);
        }
        return;
    }
    queue_.put(t, cb);
}


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
        pContext->reset = false; // Active LOW
        break;
    case CPU_SIGNAL_EXT_IRQ:
        if (pContext->reset) {
            break;
        }
        if (mstatus.bits.MIE == 0 && pContext->cur_prv_level == PRV_M) {
            // External Interrupt controller pending bit
            pContext->interrupt_pending = 1;
            break;
        }
        /// @todo delegate interrupt to non-machine privilege level.

        csr_mcause_type cause;
        cause.value     = 0;
        cause.bits.irq  = 1;
        cause.bits.code = 11;   // 11 = Machine external interrupt
        pContext->csr[CSR_mcause] = cause.value;
        pContext->interrupt = 1;
        break;
    default:
        RISCV_error("Unsupported signalRaise(%d)", idx);
    }
}

void CpuRiscV_Functional::lowerSignal(int idx) {
    CpuContextType *pContext = getpContext();
    switch (idx) {
    case CPU_SIGNAL_RESET:
        pContext->reset = true; // Active LOW
        break;
    case CPU_SIGNAL_EXT_IRQ:
        pContext->interrupt = 0;
        pContext->interrupt_pending = 0;
        break;
    default:
        RISCV_error("Unsupported lowerSignal(%d)", idx);
    }
}

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
            if (trans->write) {
                addBreakpoint(trans->wdata);
            }
            break;
        case 5:
            if (trans->write) {
                removeBreakpoint(trans->wdata);
            }
            break;
        default:;
        }
        break;
    default:;
    }
    dport.cb->nb_response_debug_port(dport.trans);
}

void 
CpuRiscV_Functional::nb_transport_debug_port(DebugPortTransactionType *trans,
                                             IDbgNbResponse *cb) {
    dport.trans = trans;
    dport.cb = cb;
    dport.valid = true;
}

void CpuRiscV_Functional::halt() {
    CpuContextType *pContext = getpContext();
    char reason[256];
    dbg_state_ = STATE_Halted;
    RISCV_sprintf(reason, sizeof(reason), 
                "[%" RV_PRI64 "d] pc:%016" RV_PRI64 "x: %08x \t CPU halted",
                getStepCounter(), pContext->pc, cacheline_[0]);

    RISCV_printf0("[%" RV_PRI64 "d] pc:%016" RV_PRI64 "x: %08x \t CPU halted",
        getStepCounter(), pContext->pc, cacheline_[0]);
}

void CpuRiscV_Functional::go() {
    dbg_state_ = STATE_Normal;
}

void CpuRiscV_Functional::step(uint64_t cnt) {
    CpuContextType *pContext = getpContext();
    dbg_step_cnt_ = pContext->step_cnt + cnt;
    dbg_state_ = STATE_Stepping;
}

uint64_t CpuRiscV_Functional::getReg(uint64_t idx) {
    CpuContextType *pContext = getpContext();
    if (idx >= 0 && idx < 32) {
        return pContext->regs[idx];
    }
    return REG_INVALID;
}

void CpuRiscV_Functional::setReg(uint64_t idx, uint64_t val) {
    CpuContextType *pContext = getpContext();
    if (idx >= 0 && idx < 32) {
        pContext->regs[idx] = val;
    }
}

uint64_t CpuRiscV_Functional::getPC() {
    CpuContextType *pContext = getpContext();
    return pContext->pc;
}

void CpuRiscV_Functional::setPC(uint64_t val) {
    CpuContextType *pContext = getpContext();
    pContext->pc = val;
}

uint64_t CpuRiscV_Functional::getNPC() {
    CpuContextType *pContext = getpContext();
    return pContext->npc;
}

void CpuRiscV_Functional::setNPC(uint64_t val) {
    CpuContextType *pContext = getpContext();
    pContext->npc = val;
}

void CpuRiscV_Functional::addBreakpoint(uint64_t addr) {
    CpuContextType *pContext = getpContext();
    pContext->ibus->addBreakpoint(addr);
}

void CpuRiscV_Functional::removeBreakpoint(uint64_t addr) {
    CpuContextType *pContext = getpContext();
    pContext->ibus->removeBreakpoint(addr);
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

}  // namespace debugger

