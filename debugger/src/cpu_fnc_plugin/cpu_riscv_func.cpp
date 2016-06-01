/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU functional simulator class definition.
 */

#include "api_core.h"
#include "cpu_riscv_func.h"
#include "riscv-isa.h"

namespace debugger {

CpuRiscV_Functional::CpuRiscV_Functional(const char *name)  
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<IHostIO *>(this));
    registerInterface(static_cast<IHap *>(this));
    registerAttribute("Bus", &bus_);
    registerAttribute("ListExtISA", &listExtISA_);
    registerAttribute("FreqHz", &freqHz_);

    bus_.make_string("");
    listExtISA_.make_list(0);
    freqHz_.make_uint64(1);

    stepQueue_.make_list(0);
    stepQueue_len_ = 0;
    cpu_context_.step_cnt = 0;

    RISCV_mutex_init(&mutexStepQueue_);
    RISCV_event_create(&config_done_, "config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
    cpu_context_.csr[CSR_mreset]   = 0;
    dbg_state_ = STATE_Normal;
    last_hit_breakpoint_ = ~0;
    reset();
}

CpuRiscV_Functional::~CpuRiscV_Functional() {
    RISCV_event_close(&config_done_);
    RISCV_mutex_destroy(&mutexStepQueue_);
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
    if ((*glb)["SimEnable"].to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void CpuRiscV_Functional::predeleteService() {
    stop();
}

void CpuRiscV_Functional::hapTriggered(EHapType type) {
    RISCV_event_set(&config_done_);
}

void CpuRiscV_Functional::busyLoop() {
    RISCV_event_wait(&config_done_);

    while (isEnabled()) {
        updatePipeline();
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

void CpuRiscV_Functional::updatePipeline() {
    IInstruction *instr;
    CpuContextType *pContext = getpContext();

    pContext->pc = pContext->npc;
    if (isRunning()) {
        fetchInstruction();
    }

    updateState();

    if (pContext->csr[CSR_mreset]) {
        queueUpdate();
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

    queueUpdate();

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

bool CpuRiscV_Functional::isRunning() {
    return  (dbg_state_ != STATE_Halted);
}

void CpuRiscV_Functional::reset() {
    CpuContextType *pContext = getpContext();
    pContext->regs[0] = 0;
    pContext->pc = RESET_VECTOR;
    pContext->npc = RESET_VECTOR;
    pContext->exception = 0;
    pContext->csr[CSR_mimpid]   = 0x0001;   // UC Berkeley Rocket repo
    pContext->csr[CSR_mheartid] = 0;
    pContext->csr[CSR_mtvec]   = 0x100;     // Hardwired RO value
    pContext->csr[CSR_mip] = 0;             // clear pending interrupts
    pContext->csr[CSR_mie] = 0;             // disabling interrupts
    pContext->csr[CSR_mepc] = 0;
    pContext->csr[CSR_mtdeleg] = 0;
    pContext->csr[CSR_mtime] = 0;
    pContext->csr[CSR_mtimecmp] = 0;
    pContext->csr[CSR_uepc] = 0;
    pContext->csr[CSR_sepc] = 0;
    pContext->csr[CSR_hepc] = 0;
    csr_mstatus_type mstat;
    mstat.value = 0;
    mstat.bits.IE = 0;
    mstat.bits.PRV = PRV_LEVEL_M;           // Current privilege level
    pContext->csr[CSR_mstatus] = mstat.value;
}

void CpuRiscV_Functional::handleTrap() {
    CpuContextType *pContext = getpContext();
    csr_mstatus_type mstatus;
    mstatus.value = pContext->csr[CSR_mstatus];

    if ((pContext->exception == 0 && pContext->csr[CSR_mip] == 0)
     || (mstatus.bits.PRV == PRV_LEVEL_M && mstatus.bits.IE == 0)) {
        return;
    }

    // All traps handle via machine mode while CSR mdelegate
    // doesn't setup other.
    // @tod delegating
    mstatus.bits.PRV3 = mstatus.bits.PRV;
    mstatus.bits.IE3 = mstatus.bits.IE;
    mstatus.bits.IE = 0;
    mstatus.bits.PRV = PRV_LEVEL_M;
    pContext->csr[CSR_mstatus] = mstatus.value;

    uint64_t xepc = (mstatus.bits.PRV << 8) + 0x41;
    if (pContext->exception) {
        pContext->csr[xepc]    = pContext->pc;
    } else {
        // Software interrupt handled after instruction was executed
        pContext->csr[xepc]    = pContext->npc;
    }
    pContext->npc = pContext->csr[CSR_mtvec] + 0x40 * mstatus.bits.PRV3;

    pContext->exception = 0;
}

void CpuRiscV_Functional::fetchInstruction() {
    CpuContextType *pContext = getpContext();
    pContext->ibus->read(pContext->pc, 
                        reinterpret_cast<uint8_t *>(cacheline_), 
                        4);
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

#if 1
static const int ra = 1;       // [1] Return address
static const int sp = 2;       // [2] Stack pointer
static const int gp = 3;       // [3] Global pointer
static const int tp = 4;       // [4] Thread pointer
static const int t0 = 5;       // [5] Temporaries 0 s3
static const int t1 = 6;       // [6] Temporaries 1 s4
static const int t2 = 7;       // [7] Temporaries 2 s5
static const int s0 = 8;       // [8] s0/fp Saved register/frame pointer
static const int s1 = 9;       // [9] Saved register 1
static const int a0 = 10;       // [10] Function argumentes 0
static const int a1 = 11;       // [11] Function argumentes 1
static const int a2 = 12;       // [12] Function argumentes 2
static const int a3 = 13;       // [13] Function argumentes 3
static const int a4 = 14;       // [14] Function argumentes 4
static const int a5 = 15;       // [15] Function argumentes 5
static const int a6 = 16;       // [16] Function argumentes 6
static const int a7 = 17;       // [17] Function argumentes 7
static const int s2 = 18;       // [18] Saved register 2
static const int s3 = 19;       // [19] Saved register 3
static const int s4 = 20;       // [20] Saved register 4
static const int s5 = 21;       // [21] Saved register 5
static const int s6 = 22;       // [22] Saved register 6
static const int s7 = 23;       // [23] Saved register 7
static const int s8 = 24;       // [24] Saved register 8
static const int s9 = 25;       // [25] Saved register 9
static const int s10 = 26;      // [26] Saved register 10
static const int s11 = 27;      // [27] Saved register 11
static const int t3 = 28;       // [28] 
static const int t4 = 29;       // [29] 
static const int t5 = 30;       // [30] 
static const int t6 = 31;      // [31] 
#endif

void CpuRiscV_Functional::executeInstruction(IInstruction *instr,
                                             uint32_t *rpayload) {

    CpuContextType *pContext = getpContext();
#if 1
    if (pContext->pc == 0x100000b4) {
        bool st = true;
    }
#endif
    instr->exec(cacheline_, pContext);
#if 0
    //if (pContext->pc >= 0x10000000) {
    if ((pContext->pc >= 0x100000b4 && pContext->pc <= 0x10000130)
    || (pContext->pc >= 0x10001ef4)
    ) {
    //if (pContext->pc >= 0x10001928 && pContext->pc <= 0x10001960) {
        RISCV_debug("[%" RV_PRI64 "d] %08x: %08x \t %4s <mstatus=%016" RV_PRI64 "x; ra=%016" RV_PRI64 "x; sp=%016" RV_PRI64 "x; tp=%016" RV_PRI64 "x>", 
            getStepCounter(),
            static_cast<uint32_t>(pContext->pc),
            rpayload[0], instr->name(),
            pContext->csr[CSR_mstatus],
            pContext->regs[ra],
            pContext->regs[sp],
            pContext->regs[tp]
            );
    }
#endif

    if (pContext->regs[0] != 0) {
        RISCV_error("Register x0 was modificated (not equal to zero)", NULL);
    }
}

void CpuRiscV_Functional::queueUpdate() {
    uint64_t ev_time;
    IClockListener *iclk;
    unsigned queue_len;
    CpuContextType *pContext = getpContext();

    // @warning We can add new event inside of stepCallback that leads to
    //          infinite cycle if wouldn't use fixed length.
    queue_len = stepQueue_len_;
    for (unsigned i = 0; i < queue_len; i++) {
        ev_time = stepQueue_[i][Queue_Time].to_uint64();

        if (pContext->step_cnt >= ev_time) {
            iclk = static_cast<IClockListener *>(
                    stepQueue_[i][Queue_IFace].to_iface());

            // remove item from list using swap function to avoid
            // allocation/deallocation calls if we can avoid it.
            RISCV_mutex_lock(&mutexStepQueue_);
            stepQueue_.swap_list_item(i, stepQueue_.size() - 1);
            stepQueue_len_--;
            queue_len--;
            i--;
            RISCV_mutex_unlock(&mutexStepQueue_);

            iclk->stepCallback(pContext->step_cnt);
        }
    }
}

void CpuRiscV_Functional::registerStepCallback(IClockListener *cb,
                                               uint64_t t) {
    AttributeType time(Attr_UInteger, t);
    AttributeType face(cb);
    RISCV_mutex_lock(&mutexStepQueue_);
    // Check if allocated queue size is greater than number of used
    // items in a list then use available item.
    if (stepQueue_len_ < stepQueue_.size()) {
        stepQueue_[stepQueue_len_][Queue_Time] = time;
        stepQueue_[stepQueue_len_][Queue_IFace] = face;
        stepQueue_len_++;
    } else {
        AttributeType item;
        item.make_list(Queue_Total);
        item[Queue_Time] = time;
        item[Queue_IFace] = face;
        stepQueue_.add_to_list(&item);
        stepQueue_len_++;
    }
    RISCV_mutex_unlock(&mutexStepQueue_);
}

uint64_t CpuRiscV_Functional::write(uint16_t adr, uint64_t val) {
    writeCSR(adr, val, getpContext());
    return 0;
}

uint64_t CpuRiscV_Functional::read(uint16_t adr, uint64_t *val) {
    *val = readCSR(adr, getpContext());
    return 0;
}

void CpuRiscV_Functional::halt() {
    CpuContextType *pContext = getpContext();
    dbg_state_ = STATE_Halted;

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
    return 0;
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

