/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System Bus class declaration (AMBA or whatever).
 */

#include "api_core.h"
#include "bus.h"
#include "coreservices/icpuriscv.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(Bus)

Bus::Bus(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IBus *>(this));
    registerAttribute("MapList", &listMap_);

    listMap_.make_list(0);
    imap_.make_list(0);
    breakpoints_.make_list(0);
}

Bus::~Bus() {
}

void Bus::postinitService() {
    IMemoryOperation *imem;
    for (unsigned i = 0; i < listMap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(RISCV_get_service_iface(
                listMap_[i].to_string(), IFACE_MEMORY_OPERATION));
        if (imem == 0) {
            RISCV_error("Can't find slave device %s", listMap_[i].to_string());
            continue;
        }
        map(imem);
    }

    AttributeType clks;
    RISCV_get_clock_services(&clks);
    if (clks.size()) {
        iclk0_ = static_cast<IClock *>(clks[0u].to_iface());
    } else {
        RISCV_error("CPUs not found", NULL);
    }
}

void Bus::map(IMemoryOperation *imemop) {
    AttributeType t1(imemop);
    imap_.add_to_list(&t1);
}

int Bus::read(uint64_t addr, uint8_t *payload, int sz) {
    IMemoryOperation *imem;
    Axi4TransactionType memop;
    bool unmapped = true;
    
    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        if (imem->getBaseAddress() <= addr
            && addr < (imem->getBaseAddress() + imem->getLength())) {

            memop.addr = addr;
            memop.rw = 0;
            memop.wstrb = 0;
            memop.xsize = sz;
            imem->transaction(&memop);
            memcpy(payload, memop.rpayload, sz);
            unmapped = false;
            break;
            /// @todo Check memory overlapping
        }
    }

    checkBreakpoint(addr);

    if (unmapped) {
        RISCV_error("[%" RV_PRI64 "d] Read from unmapped address "
                    "%08" RV_PRI64 "x", iclk0_->getStepCounter(), addr);
        
        // Stop simulation
        RISCV_break_simulation();
        RISCV_error("Simulation was OFF. HW AXI bus not available", NULL);
        return 0;
    } 

    RISCV_debug("[%08" RV_PRI64 "x] => [%08x %08x %08x %08x]",
        addr,
        memop.rpayload[3], memop.rpayload[2], 
        memop.rpayload[1], memop.rpayload[0]);
    return sz;
}

int Bus::write(uint64_t addr, uint8_t *payload, int sz) {
    IMemoryOperation *imem;
    Axi4TransactionType memop;
    bool unmapped = true;

    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        if (imem->getBaseAddress() <= addr
            && addr < (imem->getBaseAddress() + imem->getLength())) {

            memop.addr = addr;
            memop.rw = 1;
            memop.wstrb = (1 << sz) - 1;
            memop.xsize = sz;
            memcpy(memop.wpayload, payload, sz);
            imem->transaction(&memop);
            unmapped = false;
            break;
            /// @todo Check memory overlapping
        }
    }

    checkBreakpoint(addr);

    if (unmapped) {
        RISCV_error("[%" RV_PRI64 "d] Write to unmapped address "
                    "%08" RV_PRI64 "x", iclk0_->getStepCounter(), addr);
        
        // Stop simulation
        RISCV_break_simulation();
        RISCV_error("Simulation was OFF. HW AXI bus not available", NULL);
        return 0;
    }

    RISCV_debug("[%08" RV_PRI64 "x] <= [%08x %08x %08x %08x]",
        addr,
        memop.wpayload[3], memop.wpayload[2], 
        memop.wpayload[1], memop.wpayload[0]);
    return sz;
}

void Bus::addBreakpoint(uint64_t addr) {
    AttributeType br(Attr_UInteger, addr);
    for (unsigned i = 0; i < breakpoints_.size(); i++) {
        if (breakpoints_[i].to_uint64() == ~0) {
            breakpoints_[i] = br;
            return;
        }
    }
    breakpoints_.add_to_list(&br);
}

void Bus::removeBreakpoint(uint64_t addr) {
    AttributeType br(Attr_UInteger, ~0);
    for (unsigned i = 0; i < breakpoints_.size(); i++) {
        if (breakpoints_[i].to_uint64() == addr) {
            breakpoints_[i] = br;
        }
    }
}

void Bus::checkBreakpoint(uint64_t addr) {
    for (unsigned i = 0; i < breakpoints_.size(); i++) {
        if (addr == breakpoints_[i].to_uint64()) {
            AttributeType listCpu;
            RISCV_get_services_with_iface(IFACE_CPU_RISCV, &listCpu);
            for (unsigned n = 0; n < listCpu.size(); n++) {
                ICpuRiscV *iriscv = 
                    static_cast<ICpuRiscV *>(listCpu[n].to_iface());
                iriscv->hitBreakpoint(addr);
            }
        }
    }
}

}  // namespace debugger
