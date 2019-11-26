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
#include "cpu_generic.h"
#include "debug/dsumap.h"

namespace debugger {

CpuGeneric::CpuGeneric(const char *name)  
    : IService(name), IHap(HAP_ConfigDone),
    pc_(this, "pc", DSUREG(ureg.v.pc)),
    npc_(this, "npc", DSUREG(ureg.v.npc)),
    status_(this, "status", DSUREG(udbg.v.control)),
    stepping_cnt_(this, "stepping_cnt", DSUREG(udbg.v.stepping_mode_steps)),
    clock_cnt_(this, "clock_cnt", DSUREG(udbg.v.clock_cnt)),
    executed_cnt_(this, "executed_cnt", DSUREG(udbg.v.executed_cnt)),
    stackTraceCnt_(this, "stack_trace_cnt", DSUREG(ureg.v.stack_trace_cnt)),
    stackTraceBuf_(this, "stack_trace_buf", DSUREG(ureg.v.stack_trace_buf), 0),
    br_control_(this, "br_control", DSUREG(udbg.v.br_ctrl)),
    br_fetch_addr_(this, "br_fetch_addr", DSUREG(udbg.v.br_address_fetch)),
    br_fetch_instr_(this, "br_fetch_instr", DSUREG(udbg.v.br_instr_fetch)),
    br_flush_addr_(this, "br_flush_addr", DSUREG(udbg.v.br_flush_addr)),
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
    registerAttribute("GenerateRegTraceFile", &generateRegTraceFile_);
    registerAttribute("GenerateMemTraceFile", &generateMemTraceFile_);
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
    pc_z_.val = 0;
    hw_stepping_break_ = 0;
    interrupt_pending_[0] = 0;
    interrupt_pending_[1] = 0;
    sw_breakpoint_ = false;
    hw_breakpoint_ = false;
    skip_sw_breakpoint_ = false;
    hwBreakpoints_.make_list(0);
    do_not_cache_ = false;

    dport_.valid = 0;
    reg_trace_file = 0;
    mem_trace_file = 0;
    memcache_ = 0;
    memcache_flag_ = 0;
    memcache_sz_ = 0;
    fetch_addr_ = 0;
    cache_offset_ = 0;
    cachable_pc_ = false;
    CACHE_BASE_ADDR_ = 0;
    CACHE_MASK_ = 0;
    oplen_ = 0;
    RISCV_set_default_clock(static_cast<IClock *>(this));
}

CpuGeneric::~CpuGeneric() {
    RISCV_set_default_clock(0);
    RISCV_event_close(&eventConfigDone_);
    if (memcache_) {
        delete [] memcache_;
    }
    if (memcache_flag_) {
        delete [] memcache_flag_;
    }
    if (reg_trace_file) {
        reg_trace_file->close();
        delete reg_trace_file;
    }
    if (mem_trace_file) {
        mem_trace_file->close();
        delete mem_trace_file;
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
        memcache_ = new uint8_t[memcache_sz_];
        memcache_flag_ = new uint8_t[memcache_sz_];
        memset(memcache_flag_, 0, memcache_sz_);
    }

    // Get global settings:
    const AttributeType *glb = RISCV_get_global_settings();
    if ((*glb)["SimEnable"].to_bool() && isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
        if (generateRegTraceFile_.to_bool()) {
            reg_trace_file = new std::ofstream("river_func_regs.log");
        }
        if (generateMemTraceFile_.to_bool()) {
            mem_trace_file = new std::ofstream("river_func_mem.log");
        }
    }
}

void CpuGeneric::hapTriggered(IFace *isrc, EHapType type,
                                       const char *descr) {
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

    pc_.setValue(npc_.getValue());
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

        pc_z_ = pc_.getValue();
    }

    if (!branch_) {
        npc_.setValue(pc_.getValue().val + oplen_);
    }

    updateQueue();

    handleTrap();
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
            halt("Stepping breakpoint");
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
    cachable_pc_ = memcache_flag_
                && (fetch_addr_ & CACHE_MASK_) == CACHE_BASE_ADDR_;
    cache_offset_ = fetch_addr_ - CACHE_BASE_ADDR_;

    if (cachable_pc_ && memcache_flag_[cache_offset_]) {
        cacheline_[0].buf32[0] = *reinterpret_cast<uint32_t *>(
                            &memcache_[cache_offset_]);
    } else {
        trans_.action = MemAction_Read;
        trans_.addr = pc_.getValue().val;
        trans_.xsize = 4;
        trans_.wstrb = 0;
        if (dma_memop(&trans_) == TRANS_ERROR) {
            exceptionLoadInstruction(&trans_);
        }
        cacheline_[0].val = trans_.rpayload.b64[0];
        if (skip_sw_breakpoint_ && trans_.addr == br_fetch_addr_.getValue().val) {
            skip_sw_breakpoint_ = false;
            cacheline_[0].buf32[0] = br_fetch_instr_.getValue().buf32[0];
            doNotCache(trans_.addr);
        }
    }
}

void CpuGeneric::flush(uint64_t addr) {
    if (memcache_flag_ == 0) {
        return;
    }
    if (addr == ~0ull) {
        memset(memcache_flag_, 0, memcache_sz_);
    } else if ((addr & CACHE_MASK_) == CACHE_BASE_ADDR_) {
        /** SW breakpoint manager must call this flush operation */
        memcache_flag_[addr - CACHE_BASE_ADDR_] = 0;
    }
}

void CpuGeneric::trackContextEnd() {
    if (do_not_cache_) {
        if (cachable_pc_) {
            memcache_flag_[cache_offset_] = 0;
        }
    } else {
        if (icovtracker_) {
            icovtracker_->markAddress(fetch_addr_,
                                      static_cast<uint8_t>(oplen_));
        }
        if (cachable_pc_) {
            memcache_flag_[cache_offset_] = oplen_;
            *reinterpret_cast<uint32_t *>(&memcache_[cache_offset_]) =
                cacheline_[0].buf32[0];
        }
    }
    do_not_cache_ = false;
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

void CpuGeneric::setBranch(uint64_t npc) {
    branch_ = true;
    npc_.setValue(npc);
}

void CpuGeneric::pushStackTrace() {
    int cnt = static_cast<int>(stackTraceCnt_.getValue().val);
    if (cnt >= stackTraceSize_.to_int()) {
        return;
    }
    stackTraceBuf_.write(2*cnt, pc_.getValue().val);
    stackTraceBuf_.write(2*cnt + 1,  npc_.getValue().val);
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
    if (!mem_trace_file) {
    //if (!reg_trace_file) {
        return ret;
    }

    char tstr[512];
    Reg64Type pload = {0};
    if (tr->action == MemAction_Read) {
        if (tr->xsize == 4) {
            pload.buf32[0] = tr->rpayload.b32[0];
        } else {
            pload.val = tr->rpayload.b64[0];
        }
        RISCV_sprintf(tstr, sizeof(tstr),
                    "%8" RV_PRI64 "d %08x: [%08x] => %016" RV_PRI64 "x\n",
                    step_cnt_,
                    pc_.getValue().buf32[0],
                    static_cast<int>(tr->addr),
                    pload.val);
    } else {
        if (tr->xsize == 4) {
            pload.buf32[0] = tr->wpayload.b32[0];
        } else {
            pload.val = tr->wpayload.b64[0];
        }
        RISCV_sprintf(tstr, sizeof(tstr),
                    "%8" RV_PRI64 "d %08x: [%08x] <= %016" RV_PRI64 "x\n",
                    step_cnt_,
                    pc_.getValue().buf32[0],
                    static_cast<int>(tr->addr),
                    pload.val);
    }
    (*mem_trace_file) << tstr;
    mem_trace_file->flush();
    return ret;
}

void CpuGeneric::go() {
    if (estate_ == CORE_OFF) {
        RISCV_error("CPU is turned-off", 0);
        return;
    }
    estate_ = CORE_Normal;
}

void CpuGeneric::step() {
    if (estate_ == CORE_OFF) {
        RISCV_error("CPU is turned-off", 0);
        return;
    }
    hw_stepping_break_ = step_cnt_ + stepping_cnt_.getValue().val;
    estate_ = CORE_Stepping;
}

void CpuGeneric::halt(const char *descr) {
    if (estate_ == CORE_OFF) {
        RISCV_error("CPU is turned-off", 0);
        return;
    }
    char strop[32];
    uint8_t tbyte;
    unsigned bytetot = oplen_;
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
                       pc_.getValue().val, strop);
    } else {
        RISCV_info("pc:%04" RV_PRI64 "x: %s\t %s",
                       pc_.getValue().val, strop, descr);
    }
    estate_ = CORE_Halted;
    RISCV_trigger_hap(getInterface(IFACE_SERVICE), HAP_Halt, "Descr");
}

void CpuGeneric::power(EPowerAction onoff) {
    if (onoff == POWER_OFF && estate_ != CORE_OFF) {
        // Turn OFF:
        if (resetState_.is_equal("Halted")) {
            estate_ = CORE_Halted;
        } else {
            estate_ = CORE_OFF;
        }
        RISCV_trigger_hap(static_cast<IService *>(this),
                            HAP_CpuTurnOFF, "CPU Turned OFF");
    } else if (onoff == POWER_ON && estate_ == CORE_OFF) {
        // Turn ON:
        estate_ = CORE_Normal;
        RISCV_trigger_hap(static_cast<IService *>(this),
                            HAP_CpuTurnON, "CPU Turned ON");
    }
}

void CpuGeneric::reset(IFace *isource) {
    flush(~0ull);
    /** Reset address can be changed in runtime */
    pc_.setHardResetValue(getResetAddress());
    npc_.setHardResetValue(getResetAddress());
    status_.reset(isource);
    stackTraceCnt_.reset(isource);
    pc_.reset(isource);
    npc_.reset(isource);
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
    tr.addr = (static_cast<uint64_t>(trans->region) << 15) | trans->addr;
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
    uint64_t pc = pc_.getValue().val;
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
            halt("Hw breakpoint");
            return true;
        }
    }
    return false;
}

void CpuGeneric::skipBreakpoint() {
    skip_sw_breakpoint_ = true;
    sw_breakpoint_ = false;
}


uint64_t GenericStatusType::aboutToRead(uint64_t cur_val) {
    GenericCpuControlType ctrl;
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    ctrl.val = 0;
    ctrl.bits.halt = pcpu->isHalt() || !pcpu->isOn() ? 1 : 0;
    ctrl.bits.sw_breakpoint = pcpu->isSwBreakpoint() ? 1 : 0;
    ctrl.bits.hw_breakpoint = pcpu->isHwBreakpoint() ? 1 : 0;
    return ctrl.val;
}

uint64_t GenericStatusType::aboutToWrite(uint64_t new_val) {
    GenericCpuControlType ctrl;
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    ctrl.val = new_val;
    if (ctrl.bits.halt) {
        pcpu->halt("halted from DSU");
    } else if (ctrl.bits.stepping) {
        pcpu->step();
    } else {
        pcpu->go();
    }
    return new_val;
}

uint64_t FetchedBreakpointType::aboutToWrite(uint64_t new_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    pcpu->skipBreakpoint();
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

uint64_t FlushAddressType::aboutToWrite(uint64_t new_val) {
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    pcpu->flush(new_val);
    return new_val;
}

}  // namespace debugger

