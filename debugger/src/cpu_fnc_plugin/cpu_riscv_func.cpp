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
    memset(cpu_context_.csr, 0, sizeof(cpu_context_.csr));

    RISCV_event_create(&config_done_, "config_done");
    RISCV_register_hap(static_cast<IHap *>(this));
    reset();
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

bool after_reset = false;
void CpuRiscV_Functional::busyLoop() {
    CpuContextType *pContext = getpContext();
    RISCV_event_wait(&config_done_);

    while (isEnabled()) {
        pContext->step_cnt++;

        if (pContext->csr[CSR_mreset]) {
            reset();
            after_reset = true;

        } else {
            stepUpdate();
        }

        queueUpdate();
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
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
    csr_mstatus_type mstat;
    mstat.value = 0;
    mstat.bits.PRV = PRV_LEVEL_M;           // Current privilege level
    pContext->prv_last_trap = PRV_LEVEL_M;
    pContext->csr[CSR_mstatus] = mstat.value;
    pContext->prv_stack_cnt = 0;
}

void CpuRiscV_Functional::handleTrap() {
    CpuContextType *pContext = getpContext();
    csr_mstatus_type mstatus;
    mstatus.value = pContext->csr[CSR_mstatus];

    if (pContext->exception == 0 &&
        (mstatus.bits.IE == 0 || pContext->csr[CSR_mip] == 0)) {
        return;
    }

    if (pContext->prv_stack_cnt) {
        pContext->prv_stack_cnt--;
    }
    // All traps handle via machine mode while CSR mdelegate
    // doesn't setup other.
    pContext->prv_last_trap = mstatus.bits.PRV;
    uint64_t xepc = (pContext->prv_last_trap << 8) + 0x41;
    pContext->csr[xepc]    = pContext->pc;
    pContext->pc = pContext->csr[CSR_mtvec] + 0x40 * mstatus.bits.PRV;

    mstatus.bits.IE = 0;
    mstatus.bits.PRV = PRV_LEVEL_M;
    pContext->csr[CSR_mstatus] = mstatus.value;

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

void CpuRiscV_Functional::executeInstruction(IInstruction *instr,
                                             uint32_t *rpayload) {

    CpuContextType *pContext = getpContext();
    instr->exec(cacheline_, pContext);
#if 0
    if (pContext->step_cnt >= 0) {//after_reset) {
        RISCV_debug("[%" RV_PRI64 "d] %08x: %08x \t %4s <mstatus=%016" RV_PRI64 "x; ra=%016" RV_PRI64 "x; sp=%016" RV_PRI64 "x>", 
            getStepCounter(),
            static_cast<uint32_t>(pContext->pc),
            rpayload[0], instr->name(),
            pContext->csr[CSR_mstatus],
            pContext->regs[1],//ra
            pContext->regs[2]//sp
            );
    }
#endif

    if (pContext->regs[0] != 0) {
        RISCV_error("Register x0 was modificated (not equal to zero)", NULL);
    }
}

void CpuRiscV_Functional::stepUpdate() {
    CpuContextType *pContext = getpContext();
    pContext->pc = pContext->npc;

    handleTrap();

    fetchInstruction();

    IInstruction *instr = decodeInstruction(cacheline_);
    if (!instr) {
        pContext->npc += 4;
        
        RISCV_error("%08x: %08x \t unimplemented",
            static_cast<uint32_t>(pContext->pc), cacheline_[0]);
        generateException(EXCEPTION_InstrIllegal, pContext);
        return;
    }

    executeInstruction(instr, cacheline_);
}

void CpuRiscV_Functional::queueUpdate() {
    uint64_t ev_time;
    IClockListener *iclk;
    unsigned queue_len;
    CpuContextType *pContext = getpContext();

    // @warning We can add new event inside of stepCallback that leads to
    //          infinite cycle.
    queue_len = stepQueue_len_;
    for (unsigned i = 0; i < queue_len; i++) {
        ev_time = stepQueue_[i][Queue_Time].to_uint64();

        if (pContext->step_cnt >= ev_time) {
            iclk = static_cast<IClockListener *>(
                    stepQueue_[i][Queue_IFace].to_iface());
            iclk->stepCallback(pContext->step_cnt);

            // remove item from list
            stepQueue_.swap_list_item(i, stepQueue_.size() - 1);
            stepQueue_len_--;
            queue_len--;
            i--;
        }
    }
}

void CpuRiscV_Functional::registerStepCallback(IClockListener *cb,
                                               uint64_t t) {
    AttributeType time(Attr_UInteger, t);
    AttributeType face(cb);
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
}

uint64_t CpuRiscV_Functional::write(uint16_t adr, uint64_t val) {
    writeCSR(adr, val, getpContext());
    return 0;
}

uint64_t CpuRiscV_Functional::read(uint16_t adr, uint64_t *val) {
    *val = readCSR(adr, getpContext());
    return 0;
}

}  // namespace debugger

