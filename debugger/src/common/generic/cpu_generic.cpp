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

namespace debugger {

CpuGeneric::CpuGeneric(const char *name)  
    : IService(name), IHap(HAP_ConfigDone),
    portCSR_(this,  "csr",  0,     1<<12),
    portRegs_(this, "regs", 1<<12, 0x1000),
    stackTraceCnt_(this, "stack_trace_cnt", 0),
    stackTraceBuf_(this, "stack_trace_buf", 0, 0) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<ICpuFunctional *>(this));
    registerInterface(static_cast<IDPort *>(this));
    registerInterface(static_cast<IPower *>(this));
    registerInterface(static_cast<IResetListener *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("SysBus", &sysBus_);
    registerAttribute("SysBusWidthBytes", &sysBusWidthBytes_);
    registerAttribute("SourceCode", &sourceCode_);
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("DmiBAR", &dmibar_);
    registerAttribute("StackTraceSize", &stackTraceSize_);
    registerAttribute("FreqHz", &freqHz_);
    registerAttribute("GenerateTraceFile", &generateTraceFile_);
    registerAttribute("ResetVector", &resetVector_);
    registerAttribute("SysBusMasterID", &sysBusMasterID_);
    registerAttribute("CacheBaseAddress", &cacheBaseAddr_);
    registerAttribute("CacheAddressMask", &cacheAddrMask_);
    registerAttribute("CoverageTracker", &coverageTracker_);
    registerAttribute("TriggersTotal", &triggersTotal_);
    registerAttribute("McontrolMaskmax", &mcontrolMaskmax_);
    registerAttribute("ResetState", &resetState_);

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "eventConfigDone_%s", name);
    RISCV_event_create(&eventConfigDone_, tstr);
    RISCV_sprintf(tstr, sizeof(tstr), "eventDbgRequest_%s", name);
    RISCV_event_create(&eventDbgRequest_, tstr);
    RISCV_mutex_init(&mutex_csr_);
    RISCV_register_hap(static_cast<IHap *>(this));

    isysbus_ = 0;
    estate_ = CORE_OFF;
    step_cnt_ = 0;
    pc_z_ = 0;
    exceptions_ = 0;
    interrupt_pending_[0] = 0;
    interrupt_pending_[1] = 0;
    do_not_cache_ = false;
    haltreq_ = false;
    procbufexecreq_ = false;
    resumereq_ = false;
    resumeack_ = false;

    ptriggers_ = 0;
    trace_file_ = 0;
    trace_data_.step_cnt = 0;
    trace_data_.pc = 0;
    trace_data_.instrbuf.make_data(8);
    trace_data_.asmlist.make_list(1);
    memset(&trace_data_.action, 0, sizeof(trace_data_.action));
    trace_data_.action_cnt = 0;

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

    memset(ctxregs_, 0, sizeof(ctxregs_));
    PC_ = &ctxregs_[Ctx_Normal].pc.val;
    NPC_ = &ctxregs_[Ctx_Normal].npc.val;
}

CpuGeneric::~CpuGeneric() {
    RISCV_set_default_clock(0);
    RISCV_event_close(&eventConfigDone_);
    RISCV_event_close(&eventDbgRequest_);
    RISCV_mutex_destroy(&mutex_csr_);
    if (icache_) {
        delete [] icache_;
    }
    if (ptriggers_) {
        delete [] ptriggers_;
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

    stackTraceBuf_.setRegTotal(2 * stackTraceSize_.to_int());

    ptriggers_ = new TriggerStorageType[triggersTotal_.to_int()];
    memset(ptriggers_, 0, triggersTotal_.to_int()*sizeof(TriggerStorageType));

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

    setPC(getResetAddress());
    setNPC(getResetAddress());
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
    if (!updateState()) {
        return;
    }

    setPC(getNPC());
    branch_ = false;
    oplen_ = 0;

    if (!isTriggerInstruction()) {
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
        upd = false;
        if (procbufexecreq_) {
            enterProgbufExec();
            procbufexecreq_ = false;
        } else if (resumereq_) {
            resumereq_ = false;
            resumeack_ = true;
            upd = true;
            resume();
        } else {
            updateQueue();
        }
        break;
    case CORE_Normal:
        if (haltreq_) {
            haltreq_ = false;
            upd = false;
            halt(HALT_CAUSE_HALTREQ, "External Halt request");
        } else if (isTriggerICount()) {
            upd = false;
            halt(HALT_CAUSE_TRIGGER, "Trigger icount hit");
        } else if (isStepEnabled()) {
            upd = false;
            halt(HALT_CAUSE_STEP, "Stepping breakpoint");
        }
        break;
    case CORE_ProgbufExec:
        break;
    default:;
    }
    if (upd && estate_ != CORE_ProgbufExec) {
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
    bool generate_trap = false;
    fetch_addr_ = fetchingAddress();
    cachable_pc_ = false;
    instr_ = 0;

    if (estate_ == CORE_ProgbufExec) {
        memcpy(cacheline_[0].buf,
               &reinterpret_cast<uint8_t *>(progbuf_)[fetch_addr_],
               sizeof(uint32_t));
        return;
    }

    if ((fetch_addr_ & CACHE_MASK_) == CACHE_BASE_ADDR_) {
        cachable_pc_ = true;
        cache_offset_ = fetch_addr_ - CACHE_BASE_ADDR_;
        instr_ = icache_[cache_offset_].instr;
        cacheline_[0].buf32[0] = icache_[cache_offset_].buf;  // for tracer
    }

    trans_.action = MemAction_Read;
    trans_.addr = fetch_addr_;
    trans_.xsize = 4;
    trans_.wstrb = 0;
    if (dma_memop(&trans_, 1) == TRANS_ERROR) {     // x-flag
        generate_trap = true;
    } else {
        cacheline_[0].val = trans_.rpayload.b64[0];
    }

    if (generate_trap) {
        generateExceptionLoadInstruction(trans_.addr);
        handleTrap();
        setPC(getNPC());
        fetchILine();
    }
}

void CpuGeneric::handleTrap() {
    checkStackProtection();
    if (exceptions_) {
        uint64_t t = exceptions_;
        int e = 0;
        while (!(t & 0x1)) {
            t >>= 1;
            e++;
        }
        exceptions_ &= ~(1ull << e);
        handleException(e);
    } else {
        handleInterrupts();
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
    memcpy(trace_data_.instrbuf.data(), cacheline_[0].buf32, sizeof(uint32_t));
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

ETransStatus CpuGeneric::dma_memop(Axi4TransactionType *tr, int flags) {
    ETransStatus ret = TRANS_OK;
    tr->source_idx = sysBusMasterID_.to_int();
    if (isMmuEnabled()) {
        tr->addr = translateMmu(tr->addr);
    }
    if (isMpuEnabled()) {
        if (flags & 0x1) {
            if (!checkMpu(tr->addr, tr->xsize, "x")) {
                return TRANS_ERROR;
            }
        } else if (tr->action == MemAction_Write) {
            if (!checkMpu(tr->addr, tr->xsize, "w")) {
                return TRANS_ERROR;
            }
        } else {
            if (!checkMpu(tr->addr, tr->xsize, "r")) {
                return TRANS_ERROR;
            }
        }
    }
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

void CpuGeneric::resume() {
    if (estate_ == CORE_OFF) {
        RISCV_error("CPU is turned-off", 0);
    }
    estate_ = CORE_Normal;
}

void CpuGeneric::halt(uint32_t cause, const char *descr) {
    if (estate_ == CORE_OFF) {
        RISCV_error("CPU is turned-off", 0);
        return;
    }
    char strop[32];
    uint8_t tbyte;
    unsigned bytetot = oplen_;
    if (cause == HALT_CAUSE_TRIGGER || cause == HALT_CAUSE_EBREAK) {
        enterDebugMode(getPC(), cause);
    } else {
        enterDebugMode(getNPC(), cause);
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

bool CpuGeneric::isTriggerICount() {
    bool ret = false;
    TriggerStorageType *pt;
    for (unsigned i = 0; i < triggersTotal_.to_uint32(); i++) {
        pt = &ptriggers_[i];
        if (pt->data1.bitsdef.type == TriggerType_InstrCountMatch) {
            if (pt->data1.icount_bits.count - 1 == 0) {
                ret = true;
            }
            if ((pt->data1.icount_bits.count > 1) &&
                (pt->data1.icount_bits.m || pt->data1.icount_bits.s
                || pt->data1.icount_bits.u)) {
                pt->data1.icount_bits.count--;
            }
            pt->data1.icount_bits.hit = 1;
        }
    }
    return ret;
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
    //status_.reset(isource);
    if (ptriggers_) {
        memset(ptriggers_,
               0,
               triggersTotal_.to_int()*sizeof(TriggerStorageType));
    }
    stackTraceCnt_.reset(isource);
    interrupt_pending_[0] = 0;
    interrupt_pending_[1] = 0;
    do_not_cache_ = false;

    if (resetState_.is_equal("Halted")) {
        estate_ = CORE_Halted;
    } else {
        estate_ = CORE_Normal;
    }
}

bool CpuGeneric::isTriggerInstruction() {
    uint64_t pc = getPC();

    TriggerData1Type::bits_type2 *pt;
    bool fire = false;
    uint64_t action = 0;
    uint64_t mask;
    int tcnt;
    for (int i = 0; i < triggersTotal_.to_int(); i++) {
        pt = &ptriggers_[i].data1.mcontrol_bits;
        if (pt->type != TriggerType_AddrDataMatch) {
            continue;
        }
        if (!(pt->m | pt->s | pt->u)) {
            // Trigger is disabled
            continue;
        }
        if (!pt->execute) {
            continue;
        }

        switch (pt->match) {
        case 0:
            if (pc == ptriggers_[i].data2) {
                pt->hit = 1;
            }
            break;
        case 1:
            mask = 1;
            tcnt = 0;
            while ((tcnt < mcontrolMaskmax_.to_int())
                && !(ptriggers_[i].data2 & mask)) {
                mask <<= 1;
                tcnt++;
            }
            mask = ~(mask - 1);
            if ((pc & mask) == (ptriggers_[i].data2 & mask)) {
                pt->hit = 1;
            }
            break;
        case 2:
            if (pc >= ptriggers_[i].data2) {
                pt->hit = 1;
            }
            break;
        case 3:
            if (pc < ptriggers_[i].data2) {
                pt->hit = 1;
            }
            break;
        case 4:
            mask = (pc & 0xFFFFFFFFull) & (ptriggers_[i].data2 >> 32);
            if (mask == (ptriggers_[i].data2 & 0xFFFFFFFFull)) {
                pt->hit = 1;
            }
            break;
        case 5:
            mask = (pc >> 32) & (ptriggers_[i].data2 >> 32);
            if (mask == (ptriggers_[i].data2 & 0xFFFFFFFFull)) {
                pt->hit = 1;
            }
            break;
        default:;
        }

        // TODO bit 'chain'
        if (pt->hit == 1) {
            fire = true;
            action = pt->action;
        }
    }

    if (fire) {
        if (action == 0) {
            raiseSoftwareIrq();
        } else if (action == 1) {
            halt(HALT_CAUSE_TRIGGER, "Trigger instruction (hw breakpoint)");
        } else {
            RISCV_error("unsupported trigger action: %d",
                        static_cast<int>(action));
        }
    }
    return fire;
}

int CpuGeneric::resumereq() {
    if (!isHalted()) {
        return 1;
    }
    resumereq_ = true;
    resumeack_ = false;
    return 0;
}

int CpuGeneric::haltreq() {
    if (isHalted()) {
        return 1;
    }
    haltreq_ = true;
    return 0;
}


bool CpuGeneric::executeProgbuf(uint32_t *progbuf) {
    if (!isHalted()) {
        return true;
    }
    progbuf_ = progbuf;
    procbufexecreq_ = true;
    RISCV_event_clear(&eventDbgRequest_);
    RISCV_event_wait(&eventDbgRequest_);
    return false;
}

void CpuGeneric::enterProgbufExec() {
    estate_ = CORE_ProgbufExec;
    PC_ = &ctxregs_[Ctx_ProgbufExec].pc.val;
    NPC_ = &ctxregs_[Ctx_ProgbufExec].npc.val;
    *NPC_ = 0;
    RISCV_debug("%s", "Start executing progbuf");
}

void CpuGeneric::exitProgbufExec() {
    estate_ = CORE_Halted;
    PC_ = &ctxregs_[Ctx_Normal].pc.val;
    NPC_ = &ctxregs_[Ctx_Normal].npc.val;
    RISCV_debug("%s", "Ending executing progbuf");
    RISCV_event_set(&eventDbgRequest_);
}


}  // namespace debugger

