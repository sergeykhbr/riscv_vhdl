/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU functional simlator class definition.
 */

#include "api_core.h"
#include "cpu_riscv_func.h"
#include "types_amba.h"
#include "riscv-isa.h"

namespace debugger {

CpuRiscV_Functional::CpuRiscV_Functional(const char *name)  : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<ICpuRiscV *>(this));
    registerInterface(static_cast<IClock *>(this));
    registerInterface(static_cast<IHostIO *>(this));
    registerAttribute("ParentThread", &parentThread_);
    registerAttribute("PhysMem", &phys_mem_);
    registerAttribute("ListExtISA", &listExtISA_);
    registerAttribute("FreqHz", &freqHz_);

    parentThread_.make_string("");
    phys_mem_.make_string("");
    listExtISA_.make_list(0);
    freqHz_.make_uint64(1);

    stepQueue_.make_list(0);
    stepQueue_len_ = 0;
    cpu_data_.step_cnt = 0;
    memset(cpu_data_.csr, 0, sizeof(cpu_data_.csr));
    reset();
}

CpuRiscV_Functional::~CpuRiscV_Functional() {
}

void CpuRiscV_Functional::postinitService() {
    bool isEnable = false;
    IThread *iparent = static_cast<IThread *>(
       RISCV_get_service_iface(parentThread_.to_string(), IFACE_THREAD));
    if (iparent) {
        isEnable = iparent->isEnabled();
    }
    cpu_data_.imemop = static_cast<IMemoryOperation *>(
       RISCV_get_service_iface(phys_mem_.to_string(), IFACE_MEMORY_OPERATION));

    // Supported instruction sets:
    listInstr_.make_list(0);
    addIsaUserRV64I(&cpu_data_, &listInstr_);
    addIsaPrivilegedRV64I(&cpu_data_, &listInstr_);
    for (unsigned i = 0; i < listExtISA_.size(); i++) {
        if (listExtISA_[i].to_string()[0] == 'A') {
            addIsaExtensionA(&cpu_data_, &listInstr_);
        } else if (listExtISA_[i].to_string()[0] == 'F') {
            addIsaExtensionF(&cpu_data_, &listInstr_);
        } else if (listExtISA_[i].to_string()[0] == 'M') {
            addIsaExtensionM(&cpu_data_, &listInstr_);
        }
    }

    // Check hash uniquity:
    /*IInstruction *instr_i, *instr_n;
    for (unsigned i = 0; i < listInstr_.size(); i++) {
        instr_i = static_cast<IInstruction *>(listInstr_[i].to_iface());
        for (unsigned n = 0; n < listInstr_.size(); n++) {
            if (i == n) {
                continue;
            }
            instr_n = static_cast<IInstruction *>(listInstr_[n].to_iface());
            if (instr_i->hash() == instr_n->hash()) {
        
            }
        }
    }*/


    if (!cpu_data_.imemop) {
        RISCV_error("Physical Memory interface '%s' not found", 
                    phys_mem_.to_string());
        return;
    }

    if (isEnable) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void CpuRiscV_Functional::predeleteService() {
    stop();
}

bool after_reset = false;
void CpuRiscV_Functional::busyLoop() {
    // @todo config_done event callback
    RISCV_sleep_ms(200);

    while (loopEnable_) {
        cpu_data_.step_cnt++;

        if (cpu_data_.csr[CSR_mreset]) {
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
    cpu_data_.regs[0] = 0;
    cpu_data_.pc = RESET_VECTOR;
    cpu_data_.npc = RESET_VECTOR;
    cpu_data_.exception = 0;
    cpu_data_.prv_stack_cnt = 0;
    cpu_data_.csr[CSR_mip] = 0; // clear pending interrupts
    cpu_data_.csr[CSR_mie] = 0; // disabling interrupts
    csr_mstatus_type mstat;
    mstat.value = 0;
    mstat.bits.PRV = PRV_LEVEL_M;           // Current privilege level
    cpu_data_.csr[CSR_mstatus] = mstat.value;
}

void CpuRiscV_Functional::handleInterrupt() {
    if (cpu_data_.csr[CSR_mip] == 0) {
        return;
    }

    csr_mstatus_type mstatus;
    mstatus.value = cpu_data_.csr[CSR_mstatus];

    if (cpu_data_.prv_stack_cnt) {
        cpu_data_.prv_stack_cnt--;
        switch (cpu_data_.prv_stack_cnt) {
        case 0:
            cpu_data_.csr[CSR_mepc] = cpu_data_.pc;
            break;
        case 1:
            // TODO:
            break;
        case 2:
            // TODO:
            break;
        default:;
            // TODO:
        }
        cpu_data_.pc = cpu_data_.csr[CSR_mtvec] + 0x40 * mstatus.bits.PRV;
    }
}

void CpuRiscV_Functional::fetchInstruction() {
    memop_.addr = cpu_data_.pc;
    memop_.rw = 0;
    memop_.xsize = CFG_NASTI_DATA_BYTES;

    cpu_data_.imemop->transaction(&memop_);
}

IInstruction *CpuRiscV_Functional::decodeInstruction(uint32_t *rpayload) {
    IInstruction *instr = NULL;
    for (unsigned i = 0; i < listInstr_.size(); i++) {
        instr = static_cast<IInstruction *>(listInstr_[i].to_iface());
        if (instr->parse(rpayload)) {
            break;
        }
        instr = NULL;
    }

    return instr;
}

void CpuRiscV_Functional::executeInstruction(IInstruction *instr,
                                             uint32_t *rpayload) {
    instr->exec(memop_.rpayload, &cpu_data_);
#if 1
    if (after_reset) {
        RISCV_debug("[%" RV_PRI64 "d] %08x: %08x \t %4s <ra=%016" RV_PRI64 "x; sp=%016" RV_PRI64 "x>", 
            getStepCounter(),
            static_cast<uint32_t>(cpu_data_.pc),
            memop_.rpayload[0], instr->name(),
            cpu_data_.regs[1],//ra
            cpu_data_.regs[2]//sp
            );
    }
#endif

    if (cpu_data_.regs[0] != 0) {
        RISCV_error("Register x0 was modificated (not equal to zero)", NULL);
    }
}

void CpuRiscV_Functional::stepUpdate() {
    cpu_data_.pc = cpu_data_.npc;

    handleInterrupt();

    fetchInstruction();

#if 0
    if (step_cnt_ >= 33 || step_cnt_ >= 0x74a5) {
        bool st = true;
    }
#endif
    IInstruction *instr = decodeInstruction(memop_.rpayload);
    if (!instr) {
        cpu_data_.npc += 4;
        
        RISCV_error("%08x: %08x \t unimplemented",
            static_cast<uint32_t>(cpu_data_.pc), memop_.rpayload[0]);
        while (1) {}
        return;
    }

    executeInstruction(instr, memop_.rpayload);
}

void CpuRiscV_Functional::queueUpdate() {
    uint64_t ev_time;
    IClockListener *iclk;
    unsigned queue_len;

    // @warning We can add new event inside of stepCallback that leads to
    //          infinite cycle.
    queue_len = stepQueue_len_;
    for (unsigned i = 0; i < queue_len; i++) {
        ev_time = stepQueue_[i][Queue_Time].to_uint64();

        if (cpu_data_.step_cnt >= ev_time) {
            iclk = static_cast<IClockListener *>(
                    stepQueue_[i][Queue_IFace].to_iface());
            iclk->stepCallback(cpu_data_.step_cnt);

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
    writeCSR(adr, val, &cpu_data_);
    return 0;
}

uint64_t CpuRiscV_Functional::read(uint16_t adr, uint64_t *val) {
    *val = readCSR(adr, &cpu_data_);
    return 0;
}

}  // namespace debugger

