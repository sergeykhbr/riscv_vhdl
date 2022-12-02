/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plug'n'Play device functional model.
 */

#include "api_core.h"
#include "pnp.h"
#include "types_amba.h"

namespace debugger {

PNP::PNP(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("IrqController", &irqController_);
    registerAttribute("IrqId", &irqId_);
    registerAttribute("cpu_max", &cpu_max_);
    registerAttribute("l2cache_ena", &l2cache_ena_);

    memset(&regs_, 0, sizeof(regs_));
    iter_.buf = regs_.cfg_table;
    regs_.hwid = 0x20170313;
    regs_.fwid = 0;
    regs_.cfg.val = 0;
    regs_.cfg.bits.cpu_max = 1;
    regs_.cfg.bits.cfg_slots = 0;       // master + slaves + IC
    regs_.cfg.bits.l2cache_ena = 0;

    addMaster(0, VENDOR_GNSSSENSOR, RISCV_RIVER_CPU);
    addMaster(1, VENDOR_GNSSSENSOR, MST_DID_EMPTY);
    addMaster(2, VENDOR_GNSSSENSOR, GAISLER_ETH_MAC_MASTER);
    addMaster(3, VENDOR_GNSSSENSOR, GAISLER_ETH_EDCL_MASTER);
    addMaster(4, VENDOR_GNSSSENSOR, GNSSSENSOR_UART_TAP);

    addSlave(0x00010000, 512*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_BOOTROM);
    addSlave(0x08000000, 512*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_SRAM);
    addSlave(0x09000000, 256*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_FWIMAGE);
    addSlave(0x10010000, 4*1024, 1, VENDOR_GNSSSENSOR, GNSSSENSOR_UART);
    addSlave(0x10050000, 4*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_QSPI);
    addSlave(0x10060000, 4*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_GPIO);
    addSlave(0x10070000, 8*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_OTP_8KB);
    addSlave(0x10090000, 4*1024, 2, VENDOR_GNSSSENSOR, GNSSSENSOR_ETHMAC);
    addSlave(0x100f0000, 16*1024, 0, VENDOR_GNSSSENSOR, GNSS_SUB_SYSTEM);
    addSlave(0x100ff000, 4*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_PNP);
}

PNP::~PNP() {
}

void PNP::addMaster(unsigned idx, unsigned vid, unsigned did) {
    regs_.cfg.bits.cfg_slots++;
    iter_.item->vid = vid;
    iter_.item->did = did;
    iter_.item->descr.bits.descrtype = PNP_CFG_TYPE_MASTER;
    iter_.item->descr.bits.descrsize = sizeof(DeviceDescriptorType);
    iter_.item->addr_start = 0;
    iter_.item->addr_end = 0;
    iter_.buf += sizeof(DeviceDescriptorType);
    iter_.item->descr.bits.descrtype = PNP_CFG_TYPE_INVALID;
}

void PNP::addSlave(uint64_t addr, uint64_t size, unsigned irq, unsigned vid, unsigned did) {
    regs_.cfg.bits.cfg_slots++;
    iter_.item->vid = vid;
    iter_.item->did = did;
    iter_.item->descr.bits.descrtype = PNP_CFG_TYPE_SLAVE;
    iter_.item->descr.bits.descrsize = sizeof(DeviceDescriptorType);
    iter_.item->addr_start = addr;
    iter_.item->addr_end = addr + size;
    iter_.buf += sizeof(DeviceDescriptorType);
    iter_.item->descr.bits.descrtype = PNP_CFG_TYPE_INVALID;
}

void PNP::postinitService() {
    regs_.cfg.bits.cpu_max = static_cast<uint8_t>(cpu_max_.to_uint64() << 4);
    regs_.cfg.bits.l2cache_ena |= static_cast<uint8_t>(l2cache_ena_.to_uint64());
    regs_.cfg.bits.plic_irq_total =
        static_cast<uint8_t>(irqId_.to_uint64());

    iirq_ = static_cast<IIrqController *>(RISCV_get_service_iface(
        irqController_.to_string(), IFACE_IRQ_CONTROLLER));
    if (!iirq_) {
        RISCV_error("Interface IIrqController in %s not found",
                    irqController_.to_string());
    }
}

ETransStatus PNP::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((trans->addr - getBaseAddress()) & mask);
    uint8_t *mem_ = reinterpret_cast<uint8_t *>(&regs_);
    PnpMapType *pmap = reinterpret_cast<PnpMapType *>(0);
    if (trans->action == MemAction_Write) {
        for (uint64_t i = 0; i < trans->xsize; i++) {
            if ((trans->wstrb & (1 << i)) == 0) {
                continue;
            }
            if (((off + i) & ~0x3ull) == reinterpret_cast<uint64_t>(&pmap->hwid)) {
                // Trigger interrupt on write access to RO register just to check PLIC
                if (iirq_) {
                    iirq_->requestInterrupt(static_cast<IService *>(this),
                                            irqId_.to_int());
                }
                i += 3;
            } else {
                mem_[off + i] = trans->wpayload.b8[i];
            }
        }
    } else {
        for (uint64_t i = 0; i < trans->xsize; i++) {
            trans->rpayload.b8[i] = mem_[off + i];
        }
    }
    return TRANS_OK;
}

}  // namespace debugger

