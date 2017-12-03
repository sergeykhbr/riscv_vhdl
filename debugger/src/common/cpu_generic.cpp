/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU generic functional model common methods.
 */

#include <api_core.h>
#include "cpu_generic.h"
#include "coreservices/isocinfo.h"

namespace debugger {

CpuGeneric::CpuGeneric(const char *name)  
    : IService(name), IHap(HAP_ConfigDone),
    status_(this, "status", 0x10000),
    stepping_cnt_(this, "stepping_cnt", 0x10008),
    pc_(this, "pc", 0x1020 << 3),
    npc_(this, "npc", 0x1021 << 3),
    stackTraceCnt_(this, "stack_trace_cnt", 0x1022 << 3),
    stackTraceBuf_(this, "stack_trace_buf", 0x1080 << 3, 0) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<ICpuGeneric *>(this));
    registerInterface(static_cast<IResetListener *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("SysBus", &sysBus_);
    registerAttribute("DbgBus", &dbgBus_);
    registerAttribute("SourceCode", &sourceCode_);
    registerAttribute("StackTraceSize", &stackTraceSize_);
    registerAttribute("FreqHz", &freqHz_);
    registerAttribute("GenerateRegTraceFile", &generateRegTraceFile_);
    registerAttribute("GenerateMemTraceFile", &generateMemTraceFile_);
    registerAttribute("ResetVector", &resetVector_);
    registerAttribute("SysBusMasterID", &sysBusMasterID_);

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "eventConfigDone_%s", name);
    RISCV_event_create(&eventConfigDone_, tstr);
    RISCV_register_hap(static_cast<IHap *>(this));

    isysbus_ = 0;
    estate_ = CORE_OFF;
    step_cnt_ = 0;
    hw_stepping_break_ = 0;
    interrupt_pending_ = 0;

    dport_.valid = 0;
    reg_trace_file = 0;
    mem_trace_file = 0;
}

CpuGeneric::~CpuGeneric() {
    RISCV_event_close(&eventConfigDone_);
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

    stackTraceBuf_.setLength(2 * stackTraceSize_.to_int());

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

    if (!isBreakpoint()) {
        fetchILine();
        GenericInstruction *instr = decodeInstruction(cacheline_);
        if (instr) {
            oplen_ = instr->exec(cacheline_);
        } else {
            generateIllegalOpcode();
        }
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
    trans_.action = MemAction_Read;
    trans_.addr = pc_.getValue().val;
    trans_.xsize = 4;
    trans_.wstrb = 0;
    trans_.source_idx = sysBusMasterID_.to_int();
    isysbus_->b_transport(&trans_);
    cacheline_[0].val = trans_.rpayload.b64[0];
}

void CpuGeneric::registerStepCallback(IClockListener *cb,
                                               uint64_t t) {
    if (!isEnabled()) {
        if (t <= step_cnt_) {
            cb->stepCallback(t);
        }
        return;
    }
    queue_.put(t, cb);
}

void CpuGeneric::setBranch(uint64_t npc) {
    branch_ = true;
    npc_.setValue(npc);
}

void CpuGeneric::pushStackTrace() {
    int cnt = static_cast<int>(stackTraceCnt_.getValue().val);
    if (cnt < stackTraceSize_.to_int()) {
        stackTraceBuf_.write(2*cnt, pc_.getValue().val);
        stackTraceBuf_.write(2*cnt + 1,  npc_.getValue().val);
    }
    stackTraceCnt_.setValue(cnt + 1);
}

void CpuGeneric::popStackTrace() {
    uint64_t cnt = stackTraceCnt_.getValue().val;
    if (cnt) {
        stackTraceCnt_.setValue(cnt - 1);
    }
}

void CpuGeneric::dma_memop(Axi4TransactionType *tr) {
    tr->source_idx = sysBusMasterID_.to_int();
    isysbus_->b_transport(tr);
    if (!mem_trace_file) {
        return;
    }

    char tstr[512];
    if (tr->action == MemAction_Read) {
        RISCV_sprintf(tstr, sizeof(tstr),
                    "%08x: [%08x] => %016" RV_PRI64 "x\n",
                    pc_.getValue().buf32[0],
                    static_cast<int>(tr->addr),
                    tr->rpayload.b64[0]);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr),
                    "%08x: [%08x] <= %016" RV_PRI64 "x\n",
                    pc_.getValue().buf32[0],
                    static_cast<int>(tr->addr),
                    tr->wpayload.b64[0]);
    }
    (*mem_trace_file) << tstr;
    mem_trace_file->flush();
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
    for (unsigned i = 0; i < oplen_; i++) {
        RISCV_sprintf(&strop[2*i], sizeof(strop)-2*i, "%02x",
                      cacheline_[0].buf[i]);
    }
    
    if (descr == NULL) {
        RISCV_info("[%6" RV_PRI64 "d] pc:%04" RV_PRI64 "x: %s \t CPU halted",
                       step_cnt_, pc_.getValue().val, strop);
    } else {
        RISCV_info("[%6" RV_PRI64 "d] pc:%04" RV_PRI64 "x: %s\t %s",
                       step_cnt_, pc_.getValue().val, strop, descr);
    }
    estate_ = CORE_Halted;
    RISCV_trigger_hap(getInterface(IFACE_SERVICE), HAP_Halt, "Descr");
}

void CpuGeneric::reset(bool active) {
    interrupt_pending_ = 0;
    stackTraceCnt_.reset(active);
    status_.reset(active);
    pc_.setValue(resetVector_.to_uint64());
    npc_.setValue(resetVector_.to_uint64());
    if (!active && estate_ == CORE_OFF) {
        // Turn ON:
        estate_ = CORE_Normal;
        RISCV_trigger_hap(static_cast<IService *>(this),
                            HAP_CpuTurnON, "CPU Turned ON");
    } else if (active) {
        // Turn OFF:
        estate_ = CORE_OFF;
        RISCV_trigger_hap(static_cast<IService *>(this),
                            HAP_CpuTurnOFF, "CPU Turned OFF");
    }
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
    tr.addr = (static_cast<uint64_t>(trans->region) << 12) | trans->addr;
    tr.addr <<= 3;
    idbgbus_->b_transport(&tr);

    trans->rdata = tr.rpayload.b64[0];;
    dport_.cb->nb_response_debug_port(trans);

    /*DsuMapType::udbg_type::debug_region_type::control_reg ctrl;
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
            // Read only register
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
    dport.cb->nb_response_debug_port(dport.trans);*/
}

void CpuGeneric::nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb) {
    dport_.trans = trans;
    dport_.cb = cb;
    dport_.valid = true;
}

uint64_t GenericStatusType::aboutToRead(uint64_t cur_val) {
    GenericCpuControlType ctrl;
    CpuGeneric *pcpu = static_cast<CpuGeneric *>(parent_);
    ctrl.val = 0;
    ctrl.bits.halt = pcpu->isHalt() ? 1 : 0;
    ctrl.bits.breakpoint = breakpoint_ ? 1 : 0;
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

}  // namespace debugger

