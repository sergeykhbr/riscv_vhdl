#pragma once

#include "imapdev.h"

class IrqController : public IMappedDevice {
public:
    IrqController() {
        irq_lock_ = 0;
        isr_mask_ = ~0;
        isr_table_ = 0;
    }

    virtual bool isAddrValid(uint64_t addr) {
        return (addr >= 0x80002000 && addr < 0x80003000);
    }
    virtual void write(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0x80002000;
        switch (off) {
        case 0x00:
            isr_mask_ = *(uint32_t *)buf;
            break;
        case 0x08:
            isr_pending_ &= ~(*(uint32_t *)buf);
            break;
        case 0x10:
            isr_table_ = *((IsrEntryType **)buf);
            break;
        case 0x28:
            irq_lock_ = *(uint32_t *)buf;
            break;
        default:
            break;
        }

    }
    virtual void read(uint64_t addr, uint8_t *buf, int size) {
        uint64_t off = addr - 0x80002000;
        // IRQ controller
        switch (addr - 0x80002000) {
        case 0x00:
            *(uint32_t *)buf = isr_mask_;
            break;
        case 0x04:
            *(uint32_t *)buf = isr_pending_;
            break;
        case 0x10:
            *((IsrEntryType **)buf) = isr_table_;
            break;
        case 0x28:
            *(uint32_t *)buf = irq_lock_;
            break;
        case 0x2c:
            *(uint32_t *)buf = irq_cause_idx_;
            break;
        default:
            break;
        }
    }

    virtual void raise_interrupt(int idx) {
        if (!isr_table_ || irq_lock_) {
            return;
        }
        if (isr_table_[idx].func && ((isr_mask_ & (0x1 << idx)) == 0)) {
            isr_pending_ |= (1 << idx);
            isr_handler isr = (isr_handler)isr_table_[idx].func;
            irq_cause_idx_ = (uint32_t)idx;
            isr((void *)isr_table_[idx].arg);
        }
    }

private:
    typedef void (*isr_handler)(void *arg);

    struct IsrEntryType {
        uint64_t arg;
        uint64_t func;
    } *isr_table_;
    uint32_t irq_lock_;
    uint32_t isr_mask_;
    uint32_t isr_pending_;
    uint32_t irq_cause_idx_;
};

