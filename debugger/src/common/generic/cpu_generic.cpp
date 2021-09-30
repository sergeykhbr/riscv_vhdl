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
#include <generic-isa.h>
#include "cpu_generic.h"
#include "debug/dsumap.h"

namespace debugger {

CpuGeneric::CpuGeneric(const char *name)  
    : IService(name), IHap(HAP_ConfigDone),
    portRegs_(this, "regs", DSUREG(ureg.v.iregs[0]), 0x1000),    // 4096 bytes region of DSU
    dbgnpc_(this, "npc", DSUREG(csr[CSR_dpc])),
    dcsr_(this, "dcsr", DSUREG(csr[CSR_dcsr])),
    status_(this, "status", DSUREG(csr[CSR_runcontrol])),
    insperstep_(this, "insperstep", DSUREG(csr[CSR_insperstep])),
    clock_cnt_(this, "clock_cnt", DSUREG(csr[CSR_cycle])),
    executed_cnt_(this, "executed_cnt", DSUREG(csr[CSR_insret])),
    stackTraceCnt_(this, "stack_trace_cnt", DSUREG(ureg.v.stack_trace_cnt)),
    stackTraceBuf_(this, "stack_trace_buf", DSUREG(ureg.v.stack_trace_buf), 0),
    br_control_(this, "br_control", DSUREG(udbg.v.br_ctrl)),
    csr_flushi_(this, "csr_flushi", DSUREG(csr[CSR_flushi])),
    br_hw_add_(this, "br_hw_add", DSUREG(udbg.v.add_breakpoint)),
    br_hw_remove_(this, "br_hw_remove", DSUREG(udbg.v.remove_breakpoint)) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<ICpuGeneric *>(this));
    registerInterface(static_cast<ICpuFunctional *>(this));
    registerInterface(static_cast<IPower *>(this));
    registerInterface(static_cast<IResetListener *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("SysBus", &sysBus_);
    registerAttribute("DbgBus", &dbgBus_);
    registerAttribute("SysBusWidthBytes", &sysBusWidthBytes_);
    registerAttribute("SourceCode", &sourceCode_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("Tap", &tap_);
    registerAttribute("StackTraceSize", &stackTraceSize_);
    registerAttribute("FreqHz", &freqHz_);
    registerAttribute("GenerateTraceFile", &generateTraceFile_);
    registerAttribute("ResetVector", &resetVector_);
    registerAttribute("SysBusMasterID", &sysBusMasterID_);
    registerAttribute("CacheBaseAddress", &cacheBaseAddr_);
    registerAttribute("CacheAddressMask", &cacheAddrMask_);
    registerAttribute("CoverageTracker", &coverageTracker_);
    registerAttribute("ResetState", &resetState_);

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "eventConfigDone_%s", name);
    RISCV_event_create(&eventConfigDone_, tstr);
    RISCV_register_hap(static_cast<IHap *>(this));

    isysbus_ = 0;
    estate_ = CORE_OFF;
    step_cnt_ = 0;
    pc_z_ = 0;
    hw_stepping_break_ = 0;
    interrupt_pending_[0] = 0;
    interrupt_pending_[1] = 0;
    sw_breakpoint_ = false;
    hw_breakpoint_ = false;
    hwBreakpoints_.make_list(0);
    do_not_cache_ = false;

    dport_.valid = 0;
    trace_file_ = 0;
    memset(&trace_data_, 0, sizeof(trace_data_));
    icache_ = 0;
    memcache_sz_ = 0;
    fetch_addr_ = 0;
    cache_offset_ = 0;
    cachable_pc_ = false;
    CACHE_BASE_ADDR_ = 0;
    CACHE_MASK_ = 0;
    oplen_ = 0;
    RISCV_set_default_clock(static_cast<IClock *>(this));

    R = portRegs_.getpR64();
    PC_ = &R[33];       // as mapped in dsu by default
    NPC_ = &R[34];       // as mapped in dsu by default
}

CpuGeneric::~CpuGeneric() {
    RISCV_set_default_clock(0);
    RISCV_event_close(&eventConfigDone_);
    if (icache_) {
        delete [] icache_;
    }
    if (trace_file_) {
        trace_file_->close();
        delete trace_file_;
    }
}

void CpuGeneric::postinitService() {
    if (resetState_.is_equal("Halted")) {
        estate_ = CORE_Halted;
    } else {
        estate_ = CORE_OFF;
    }

    isysbus_ = static_cast<IMemoryOperation *>(
        RISCV_get_service_iface(sysBus_.to_string(), IFACE_MEMORY_OPERATION));
    if (!isysbus_) {
        RISCV_error("System Bus interface '%s' not found",
                    sysBus_.to_string());
        return;
    }

    idbgbus_ = static_cast<IMemoryOperation *>(
        RISCV_get_service_iface(dbgBus_.to_string(), IFACE_MEMORY_OPERATION));
    if (!idbgbus_) {
        RISCV_error("Debug Bus interface '%s' not found",
                    dbgBus_.to_string());
        return;
    }

    isrc_ = static_cast<ISourceCode *>(
       RISCV_get_service_iface(sourceCode_.to_string(), IFACE_SOURCE_CODE));
    if (!isrc_) {
        RISCV_error("Source code interface '%s' not found", 
                    sourceCode_.to_string());
        return;
    }

    icovtracker_ = 0;
    if (coverageTracker_.size()) {
        icovtracker_ = static_cast<ICoverageTracker *>(
            RISCV_get_service_iface(coverageTracker_.to_string(),
                                    IFACE_COVERAGE_TRACKER));
        if (!icovtracker_) {
            RISCV_error("ICoverageTracker interface '%s' not found", 
                        coverageTracker_.to_string());
        }
    }

    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
        return;
    }

    itap_ = static_cast<ITap *>(
       RISCV_get_service_iface(tap_.to_string(), IFACE_TAP));
    if (!itap_) {
        RISCV_error("ITap interface '%s' not found", tap_.to_string());
        return;
    }

    stackTraceBuf_.setRegTotal(2 * stackTraceSize_.to_int());

    CACHE_BASE_ADDR_ = cacheBaseAddr_.to_uint64();
    CACHE_MASK_ = ~cacheAddrMask_.to_uint64();
    if (cacheAddrMask_.to_uint64()) {
        memcache_sz_ = cacheAddrMask_.to_int() + 1;
        icache_ = new ICacheType[memcache_sz_];
        memset(icache_, 0, memcache_sz_*sizeof(ICacheType));
    }

    // Get global settings:
    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["SimEnable"].to_bool() && isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
        if (generateTraceFile_.is_string() && generateTraceFile_.size()) {
            trace_file_ = new std::ofstream(generateTraceFile_.to_string());
        }
    }
}

void CpuGeneric::hapTriggered(EHapType type,
                              uint64_t param,
                              const char *descr) {
	RISCV_unregister_hap(static_cast<IHap *>(this));
    RISCV_event_set(&eventConfigDone_);
}

void CpuGeneric::busyLoop() {
    RISCV_event_wait(&eventConfigDone_);

    while (isEnabled()) {
        updatePipeline();
    }
}

void CpuGeneric::updatePipeline() {
    if (dport_.valid) {
        dport_.valid = 0;
        updateDebugPort();
    }

    if (!updateState()) {
        return;
    }

    setPC(getNPC());
    branch_ = false;
    oplen_ = 0;

    if (!checkHwBreakpoint()) {
        fetchILine();
        instr_ = decodeInstruction(cacheline_);

        trackContextStart();
        if (instr_) {
            oplen_ = instr_->exec(cacheline_);
        } else {
            generateIllegalOpcode();
        }
        trackContextEnd();

        pc_z_ = getPC();
    }

    if (!branch_) {
        setNPC(getPC() + oplen_);
    }

    updateQueue();

    handleTrap();

    if (trace_file_) {
        traceOutput();
    }
}

bool CpuGeneric::updateState() {
    bool upd = true;
    switch (estate_) {
    case CORE_OFF:
    case CORE_Halted:
        updateQueue();
        upd = false;
        break;
    case CORE_Stepping:
        if (hw_stepping_break_ <= step_cnt_) {
            halt(HaltStepping, "Stepping breakpoint");
            upd = false;
        }
        break;
    default:;
    }
    if (upd) {
        step_cnt_++;
    }
    return upd;
}

void CpuGeneric::updateQueue() {
    IFace *cb;
    queue_.initProc();
    queue_.pushPreQueued();

    while ((cb = queue_.getNext(step_cnt_)) != 0) {
        static_cast<IClockListener *>(cb)->stepCallback(step_cnt_);
    }
}

void CpuGeneric::fetchILine() {
    fetch_addr_ = fetchingAddress();
    cachable_pc_ = false;
    instr_ = 0;
    if ((fetch_addr_ & CACHE_MASK_) == CACHE_BASE_ADDR_) {
        cachable_pc_ = true;
        cache_offset_ = fetch_addr_ - CACHE_BASE_ADDR_;
        instr_ = icache_[cache_offset_].instr;
        cacheline_[0].buf32[0] = icache_[cache_offset_].buf;  // for tracer
    }
    

    if (!instr_) {
        trans_.action = MemAction_Read;
        trans_.addr = getPC();
        trans_.xsize = 4;
        trans_.wstrb = 0;
        if (dma_memop(&trans_) == TRANS_ERROR) {
            exceptionLoadInstruction(&trans_);
            uint64_t t_pc = fetchingAddress();
            handleTrap();
            setPC(getNPC());
            t_pc = fetchingAddress();
            fetchILine();
        } else {
            cacheline_[0].val = trans_.rpayload.b64[0];
        }
    }
}

void CpuGeneric::flush(uint64_t addr) {
    if (icache_ == 0) {
        return;
    }
    if (addr == ~0ull) {
        memset(icache_, 0, memcache_sz_*sizeof(ICacheType));
    } else if ((addr & CACHE_MASK_) == CACHE_BASE_ADDR_) {
        /** SW breakpoint manager must call this flush operation */
        if ((addr & CACHE_MASK_) == CACHE_BASE_ADDR_) {
            icache_[addr - CACHE_BASE_ADDR_].instr = 0;
        }
    }
}

void CpuGeneric::trackContextStart() {
    if (!trace_file_) {
        return;
    }
    trace_data_.action_cnt = 0;
    trace_data_.step_cnt = step_cnt_;
    trace_data_.pc = getPC();
    trace_data_.instr = cacheline_[0].buf32[0];
}

void CpuGeneric::trackContextEnd() {
    if (do_not_cache_) {
        if (cachable_pc_) {
            icache_[cache_offset_].instr = 0;
        }
    } else {
        if (icovtracker_) {
            icovtracker_->markAddress(fetch_addr_,
                                      static_cast<uint8_t>(oplen_));
        }
        if (cachable_pc_) {
            icache_[cache_offset_].instr = instr_;
            icache_[cache_offset_].buf = cacheline_[0].buf32[0];
        }
    }
    do_not_cache_ = false;
}

void CpuGeneric::traceRegister(int idx, uint64_t v) {
    if (trace_data_.action_cnt >= 64) {
        return;
    }
    trace_action_type *p = &trace_data_.action[trace_data_.action_cnt++];
    p->memop = false;
    p->waddr = idx;
    p->wdata = v;
}

void CpuGeneric::traceMemop(uint64_t addr, int we, uint64_t v, uint32_t sz) {
    if (trace_data_.action_cnt >= 64) {
        return;
    }
    trace_action_type *p = &trace_data_.action[trace_data_.action_cnt++];
    p->memop = true;
    p->memop_addr = addr;
    p->memop_write = we;
    p->memop_data.val = v;
    p->memop_size = sz;
}

void CpuGeneric::registerStepCallback(IClockListener *cb,
                                               uint64_t t) {
    if (!isEnabled() && t <= step_cnt_) {
        cb->stepCallback(t);
        return;
    }
    queue_.put(t, cb);
}

bool CpuGeneric::moveStepCallback(IClockListener *cb, uint64_t t) {
    if (queue_.move(cb, t)) {
        return true;
    }
    registerStepCallback(cb, t);
    return false;
}

void CpuGeneric::setReg(int idx, uint64_t val) {
    R[idx] = val;
    if (trace_file_) {
        traceRegister(idx, val);
    }
}

void CpuGeneric::setBranch(uint64_t npc) {
    branch_ = true;
    setNPC(npc);
}

void CpuGeneric::pushStackTrace() {
    int cnt = static_cast<int>(stackTraceCnt_.getValue().val);
    if (cnt >= stackTraceSize_.to_int()) {
        return;
    }
    stackTraceBuf_.write(2*cnt, getPC());
    stackTraceBuf_.write(2*cnt + 1, getNPC());
    stackTraceCnt_.setValue(cnt + 1);
}

void CpuGeneric::popStackTrace() {
    uint64_t cnt = stackTraceCnt_.getValue().val;
    if (cnt) {
        stackTraceCnt_.setValue(cnt - 1);
    }
}

ETransStatus CpuGeneric::dma_memop(Axi4TransactionType *tr) {
    ETransStatus ret = TRANS_OK;
    tr->source_idx = sysBusMasterID_.to_int();
    if (tr->xsize <= sysBusWidthBytes_.to_uint32()) {
        ret = isysbus_->b_transport(tr);
    } else {
        // 1-byte access for HC08
        Axi4TransactionType tr1 = *tr;
        unsigned minsz = sysBusWidthBytes_.to_uint32();
        tr1.xsize = minsz;
        tr1.wstrb = (1 << minsz) - 1;
        for (unsigned i = 0; i < tr->xsize; i+=minsz) {
            tr1.addr = tr->addr + i;
            if (tr->action == MemAction_Write) {
                memcpy(tr1.wpayload.b8, &tr->wpayload.b8[i], minsz);
            }
            ret = isysbus_->b_transport(&tr1);
            if (tr->action == MemAction_Read) {
                memcpy(&tr->rpayload.b8[i], tr1.rpayload.b8, minsz);
            }
        }
    }

    if (trace_file_) {
        int we = tr->action == MemAction_Write ? 1 : 0;
        Reg64Type memop_data;
        memop_data.val = 0;
        if (tr->action == MemAction_Read) {
            memcpy(memop_data.buf, tr->rpayload.b8, tr->xsize);
        } else {
            memcpy(memop_data.buf, tr->wpayload.b8, tr->xsize);
        }
        traceMemop(tr->addr, we,  memop_data.val, tr->xsize);
    }
    return ret;
}

void CpuGeneric::go() {
    if (estate_ == CORE_OFF) {
        RISCV_error("CPU is turned-off", 0);
        return;
    }
    if (dcsr_.isSteppingMode()) {
        dcsr_.clearSteppingMode();
        hw_stepping_break_ = step_cnt_ + insperstep_.getValue().val;
        estate_ = CORE_Stepping;
    } else {
        estate_ = CORE_Normal;
    }
}

void CpuGeneric::halt(EHaltCause cause, const char *descr) {
    if (estate_ == CORE_OFF) {
        RISCV_error("CPU is turned-off", 0);
        return;
    }
    char strop[32];
    uint8_t tbyte;
    unsigned bytetot = oplen_;
    if (cause != HaltDoNotChange) {
        dcsr_.setHaltCause(cause);
    }

    if (!bytetot) {
        bytetot = 1;
    }
    for (unsigned i = 0; i < bytetot; i++) {
        if (endianess() == LittleEndian) {
            tbyte = cacheline_[0].buf[bytetot-i-1];
        } else {
            tbyte = cacheline_[0].buf[i];
        }
        RISCV_sprintf(&strop[2*i], sizeof(strop)-2*i, "%02x", tbyte);
    }
    
    if (descr == NULL) {
        RISCV_info("pc:%04" RV_PRI64 "x: %s \t CPU halted",
                       getPC(), strop);
    } else {
        RISCV_info("pc:%04" RV_PRI64 "x: %s\t %s",
                       getPC(), strop, descr);
    }
    estate_ = CORE_Halted;
}

void CpuGeneric::power(EPowerAction onoff) {
    if (onoff == POWER_OFF && estate_ != CORE_OFF) {
        // Turn OFF:
        if (resetState_.is_equal("Halted")) {
            estate_ = CORE_Halted;
        } else {
            estate_ = CORE_OFF;
        }
        RISCV_trigger_hap(HAP_CpuTurnOFF, 0, "CPU Turned OFF");
    } else if (onoff == POWER_ON && estate_ == CORE_OFF) {
        // Turn ON:
        estate_ = CORE_Normal;
        RISCV_trigger_hap(HAP_CpuTurnON, 0, "CPU Turned ON");
    }
}

void CpuGeneric::reset(IFace *isource) {
    flush(~0ull);
    /** Reset address can be changed in runtime */
    portRegs_.reset();
    setPC(getResetAddress());
    setNPC(getResetAddress());
    status_.reset(isource);
    stackTraceCnt_.reset(isource);
    interrupt_pending_[0] = 0;
    interrupt_pending_[1] = 0;
    hw_breakpoint_ = false;
    sw_breakpoint_ = false;
    do_not_cache_ = false;
}

void CpuGeneric::updateDebugPort() {
    DebugPortTransactionType *trans = dport_.trans;
    Axi4TransactionType tr;
    tr.xsize = 8;
    tr.source_idx = 0;
    if (trans->write) {
        tr.action = MemAction_Write;
        tr.wpayload.b64[0] = trans->wdata;
        tr.wstrb = 0xFF;
    } else {
        tr.action = MemAction_Read;
        tr.rpayload.b64[0] = 0;
    }
    tr.addr = static_cast<uint64_t>(trans->addr) << 3;
    idbgbus_->b_transport(&tr);

    trans->rdata = tr.rpayload.b64[0];
    dport_.cb->nb_response_debug_port(trans);
}

void CpuGeneric::nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb) {
    dport_.trans = trans;
    dport_.cb = cb;
    dport_.valid = true;
}

void CpuGeneric::addHwBreakpoint(uint64_t addr) {
    AttributeType item;
    item.make_uint64(addr);
    hwBreakpoints_.add_to_list(&item);
    hwBreakpoints_.sort();
    for (unsigned i = 0; i < hwBreakpoints_.size(); i++) {
        RISCV_debug("Breakpoint[%d]: 0x%04" RV_PRI64 "x",
                    i, hwBreakpoints_[i].to_uint64());
    }
    flush(addr);
}

void CpuGeneric::removeHwBreakpoint(uint64_t addr) {
    for (unsigned i = 0; i < hwBreakpoints_.size(); i++) {
        if (addr == hwBreakpoints_[i].to_uint64()) {
            hwBreakpoints_.remove_from_list(i);
            hwBreakpoints_.sort();
            return;
        }
    }
    flush(addr);
}

bool CpuGeneric::checkHwBreakpoint() {
    uint64_t pc = getPC();
    if (hw_breakpoint_ && pc == hw_break_addr_) {
        hw_breakpoint_ = false;
        return false;
    }
    hw_breakpoint_ = false;

    for (unsigned i = 0; i < hwBreakpoints_.size(); i++) {
        uint64_t bradr = hwBreakpoints_[i].to_uint64();
        if (pc < bradr) {
            // Sorted list
            break;
        }
        if (pc == bradr) {
            hw_break_addr_ = pc;
            hw_breakpoint_ = true;
            halt(HaltHwTrigger, "Hw breakpoint");
            return true;
        }
    }
    return false;
}

uint64_t GenericNPCType::aboutToRead(uint64_t cur_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    return pcpu->getNPC();
}

uint64_t GenericNPCType::aboutToWrite(uint64_t new_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    pcpu->setNPC(new_val);
    return new_val;
}

uint64_t CsrDebugStatusType::aboutToWrite(uint64_t new_val) {
    // todo: select enter Debug mode or not
    return new_val;
}

uint64_t GenericStatusType::aboutToWrite(uint64_t new_val) {
    CrGenericRuncontrolType runctrl;
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    runctrl.val = new_val;
    if (runctrl.bits.req_halt) {
        pcpu->halt(HaltExternal, "halted from DSU");
    } else if (runctrl.bits.req_resume) {
        pcpu->go();
    }
    return new_val;
}

uint64_t CsrFlushiType::aboutToWrite(uint64_t new_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    pcpu->flush(new_val);
    return new_val;
}

uint64_t AddBreakpointType::aboutToWrite(uint64_t new_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    pcpu->addHwBreakpoint(new_val);
    return new_val;
}

uint64_t RemoveBreakpointType::aboutToWrite(uint64_t new_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    pcpu->removeHwBreakpoint(new_val);
    return new_val;
}

uint64_t StepCounterType::aboutToRead(uint64_t cur_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    return pcpu->getStepCounter();
}

}  // namespace debugger

