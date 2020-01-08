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
    registerAttribute("Tech", &tech_);
    registerAttribute("AdcDetector", &adc_detector_);

    tech_.make_uint64(0);
    adc_detector_.make_uint64(0);

    memset(&regs_, 0, sizeof(regs_));
    iter_.buf = regs_.cfg_table;
    regs_.hwid = 0x20170313;
    regs_.fwid = 0;
    regs_.tech.bits.tech = TECH_INFERRED;
    regs_.tech.bits.mst_total = 0;
    regs_.tech.bits.slv_total = 0;

    addMaster(0, VENDOR_GNSSSENSOR, RISCV_RIVER_CPU);
    addMaster(1, VENDOR_GNSSSENSOR, MST_DID_EMPTY);
    addMaster(2, VENDOR_GNSSSENSOR, GAISLER_ETH_MAC_MASTER);
    addMaster(3, VENDOR_GNSSSENSOR, GAISLER_ETH_EDCL_MASTER);
    addMaster(4, VENDOR_GNSSSENSOR, GNSSSENSOR_UART_TAP);

    addSlave(0, 32*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_BOOTROM);
    addSlave(0x00010000, 8*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_OTP_8KB);
    addSlave(0x00100000, 256*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_FWIMAGE);
    addSlave(0x00200000, 256*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_SPI_FLASH);
    addSlave(0x10000000, 512*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_SRAM);
    addSlave(0x80000000, 4*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_GPIO);
    addSlave(0x80001000, 4*1024, 1, VENDOR_GNSSSENSOR, GNSSSENSOR_UART);
    addSlave(0x80002000, 4*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_IRQCTRL);
    addSlave(0x80005000, 4*1024, 3, VENDOR_GNSSSENSOR, GNSSSENSOR_GPTIMERS);
    addSlave(0x80006000, 4*1024, 2, VENDOR_GNSSSENSOR, GNSSSENSOR_ETHMAC);
    addSlave(0x80007000, 4*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_DSU);
    addSlave(0x80008000, 32*1024, 0, VENDOR_GNSSSENSOR, GNSS_SUB_SYSTEM);
    addSlave(0xfffff000, 4*1024, 0, VENDOR_GNSSSENSOR, GNSSSENSOR_PNP);
}

PNP::~PNP() {
}

void PNP::addMaster(unsigned idx, unsigned vid, unsigned did) {
    regs_.tech.bits.mst_total++;
    iter_.item->mst.vid = vid;
    iter_.item->mst.did = did;
    iter_.item->mst.descr.bits.descrtype = PNP_CFG_TYPE_MASTER;
    iter_.item->mst.descr.bits.descrsize = sizeof(MasterConfigType);
    iter_.buf += sizeof(MasterConfigType);
}

void PNP::addSlave(uint64_t addr, uint64_t size, unsigned irq, unsigned vid, unsigned did) {
    regs_.tech.bits.slv_total++;
    iter_.item->slv.vid = vid;
    iter_.item->slv.did = did;
    iter_.item->slv.descr.bits.descrtype = PNP_CFG_TYPE_SLAVE;
    iter_.item->slv.descr.bits.descrsize = sizeof(SlaveConfigType);
    iter_.item->slv.descr.bits.bar_total = 1;
    iter_.item->slv.descr.bits.irq_idx = irq;
    iter_.item->slv.xaddr = static_cast<uint32_t>(addr);
    iter_.item->slv.xmask = static_cast<uint32_t>(~(size - 1));
    iter_.buf += sizeof(SlaveConfigType);
}

void PNP::postinitService() {
    regs_.tech.bits.tech = static_cast<uint8_t>(tech_.to_uint64());
    regs_.tech.bits.adc_detect =
        static_cast<uint8_t>(adc_detector_.to_uint64());
}

ETransStatus PNP::b_transport(Axi4TransactionType *trans) {
    uint64_t mask = (length_.to_uint64() - 1);
    uint64_t off = ((trans->addr - getBaseAddress()) & mask);
    uint8_t *mem_ = reinterpret_cast<uint8_t *>(&regs_);
    if (trans->action == MemAction_Write) {
        for (uint64_t i = 0; i < trans->xsize; i++) {
            if ((trans->wstrb & (1 << i)) == 0) {
                continue;
            }
            mem_[off + i] = trans->wpayload.b8[i];
        }
    } else {
        for (uint64_t i = 0; i < trans->xsize; i++) {
            trans->rpayload.b8[i] = mem_[off + i];
        }
    }
    return TRANS_OK;
}

}  // namespace debugger

