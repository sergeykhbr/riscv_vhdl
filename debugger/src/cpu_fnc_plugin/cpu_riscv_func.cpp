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
    cpu_context_.stack_trace_cnt = 0;

    RISCV_event_create(&config_done_, "riscv_func_config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
    cpu_context_.reset   = true;
    dbg_state_ = STATE_Normal;
    last_hit_breakpoint_ = ~0;
    reset();

    trans_.source_idx = CFG_NASTI_MASTER_CACHED;
    cpu_context_.reg_trace_file = 0;
    cpu_context_.mem_trace_file = 0;
    dport.valid = 0;
}

CpuRiscV_Functional::~CpuRiscV_Functional() {
    CpuContextType *pContext = getpContext();
    if (pContext->reg_trace_file) {
        pContext->reg_trace_file->close();
        delete pContext->reg_trace_file;
    }
    if (pContext->mem_trace_file) {
        pContext->mem_trace_file->close();
        delete pContext->mem_trace_file;
    }
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
    csr_mcause_type mcause;
    mstatus.value = pContext->csr[CSR_mstatus];
    mcause.value =  pContext->csr[CSR_mcause];

    if (pContext->exception == 0 && pContext->interrupt == 0) {
        return;
    }
    if (pContext->interrupt && 
        mstatus.bits.MIE == 0 && pContext->cur_prv_level == PRV_M) {
        return;
    }
    if (mcause.value == EXCEPTION_Breakpoint
        && pContext->br_ctrl.bits.trap_on_break == 0) {
        pContext->exception = 0;
        pContext->npc = pContext->pc;
        halt("EBREAK Breakpoint");
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
    pContext->br_ctrl.val = 0;
    pContext->br_inject_fetch = false;
    pContext->br_status_ena = false;
    pContext->stack_trace_cnt = 0;
}

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

void CpuRiscV_Functional::debugRegOutput(const char *marker,
                                         CpuContextType *pContext) {
        RISCV_debug("%s[%" RV_PRI64 "d] %d %08x: "
                    "1:%016" RV_PRI64 "x; 2:%016" RV_PRI64 "x; 3:%016" RV_PRI64 "x; 4:%016" RV_PRI64 "x " 
                    "5:%016" RV_PRI64 "x; 6:%016" RV_PRI64 "x; 7:%016" RV_PRI64 "x; 8:%016" RV_PRI64 "x " 
                    "9:%016" RV_PRI64 "x; 10:%016" RV_PRI64 "x; 11:%016" RV_PRI64 "x; 12:%016" RV_PRI64 "x "
                    "13:%016" RV_PRI64 "x; 14:%016" RV_PRI64 "x; 15:%016" RV_PRI64 "x; 16:%016" RV_PRI64 "x "
                    "17:%016" RV_PRI64 "x; 18:%016" RV_PRI64 "x; 19:%016" RV_PRI64 "x; 20:%016" RV_PRI64 "x " 
                    "21:%016" RV_PRI64 "x; 22:%016" RV_PRI64 "x; 24:%016" RV_PRI64 "x; 25:%016" RV_PRI64 "x " 
                    "25:%016" RV_PRI64 "x; 26:%016" RV_PRI64 "x; 27:%016" RV_PRI64 "x; 28:%016" RV_PRI64 "x " 
                    "29:%016" RV_PRI64 "x; 30:%016" RV_PRI64 "x; 31:%016" RV_PRI64 "x",
            marker,
            getStepCounter(),
            (int)pContext->cur_prv_level,
            (uint32_t)pContext->csr[CSR_mepc],
            pContext->regs[1], pContext->regs[2], pContext->regs[3], pContext->regs[4],
            pContext->regs[5], pContext->regs[6], pContext->regs[7], pContext->regs[8],
            pContext->regs[9], pContext->regs[10], pContext->regs[11], pContext->regs[12],
            pContext->regs[13], pContext->regs[14], pContext->regs[15], pContext->regs[16],
            pContext->regs[17], pContext->regs[18], pContext->regs[19], pContext->regs[20],
            pContext->regs[21], pContext->regs[22], pContext->regs[23], pContext->regs[24],
            pContext->regs[25], pContext->regs[26], pContext->regs[27], pContext->regs[28],
            pContext->regs[29], pContext->regs[30], pContext->regs[31]);
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
    if (pContext->pc == 0x10000148 && pContext->regs[Reg_a0] != 3) {
        RISCV_debug("IstWrapper [%" RV_PRI64 "d]: idx=%" RV_PRI64 "d",
            getStepCounter(), pContext->regs[Reg_a0]);
    }
    if (pContext->pc == 0x1000246c) {
        RISCV_debug("new_thread [%" RV_PRI64 "d]: "
                    "Stack:%016" RV_PRI64 "x; StackSize:%" RV_PRI64 "d; Entry:%08" RV_PRI64 "x " 
                    "tp:%016" RV_PRI64 "x => %016" RV_PRI64 "x; prio=%" RV_PRI64 "d",
            getStepCounter(),
            pContext->regs[Reg_a0], pContext->regs[Reg_a1], pContext->regs[Reg_a2],
            pContext->regs[Reg_tp], pContext->regs[Reg_a0]+96,
            pContext->regs[Reg_a6]);
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
        case 22080:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'h';
            msg_len = 1;
            break;
        case 22500:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'e';
            msg[1] = 'l';
            msg_len = 2;
            break;
        case 24350:
            uart = static_cast<IService *>(RISCV_get_service("uart0"));
            msg[0] = 'p';
            msg[1] = '\r';
            msg[2] = '\n';
            msg_len = 3;
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

void 
CpuRiscV_Functional::nb_transport_debug_port(DebugPortTransactionType *trans,
                                             IDbgNbResponse *cb) {
    dport.trans = trans;
    dport.cb = cb;
    dport.valid = true;
}

void CpuRiscV_Functional::halt(const char *descr) {
    CpuContextType *pContext = getpContext();
    dbg_state_ = STATE_Halted;

    if (descr == NULL) {
        RISCV_printf0(
            "[%" RV_PRI64 "d] pc:%016" RV_PRI64 "x: %08x \t CPU halted",
            getStepCounter(), pContext->pc, cacheline_[0]);
    } else {
        RISCV_printf0("[%" RV_PRI64 "d] pc:%016" RV_PRI64 "x: %08x \t %s",
            getStepCounter(), pContext->pc, cacheline_[0], descr);
    }
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
    //CpuContextType *pContext = getpContext();
}

void CpuRiscV_Functional::removeBreakpoint(uint64_t addr) {
    //CpuContextType *pContext = getpContext();
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

