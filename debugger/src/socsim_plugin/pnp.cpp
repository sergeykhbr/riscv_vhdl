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

static const PnpMapType DEFAULT_CONFIG = {
        0x20161011, 0xdeadcafe,
        {TECH_INFERRED, CFG_NASTI_SLAVES_TOTAL, 0, 0xff},
        0,
        0,
        0,//malloc_addr
        0,
        0,
        {0,0},
        {
          {0xffffe000, 0,          GNSSSENSOR_BOOTROM,      VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},
          {0xfffc0000, 0x00100000, GNSSSENSOR_FWIMAGE,      VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},
          {0xfff80000, 0x10000000, GNSSSENSOR_SRAM,         VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},
          {0xfffff000, 0x80001000, GNSSSENSOR_UART,         VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},
          {0xfffff000, 0x80000000, GNSSSENSOR_GPIO,         VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},
          {0xfffff000, 0x80002000, GNSSSENSOR_IRQCTRL,      VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},
          {0xfffff000, 0x80003000, GNSSSENSOR_ENGINE_STUB,  VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},
          {0xfffff000, 0x00000000, GNSSSENSOR_DUMMY,        VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},//rfctrl
          {0xfffff000, 0x00000000, GNSSSENSOR_DUMMY,        VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},//fse
          {0xfffff000, 0x00000000, GNSSSENSOR_DUMMY,        VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},//mac
          {0xfffff000, 0x00000000, GNSSSENSOR_DUMMY,        VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES},//dsu
          {0xfffff000, 0xfffff000, GNSSSENSOR_PNP,          VENDOR_GNSSSENSOR, PNP_CONFIG_DEFAULT_BYTES}
        }
    };

PNP::PNP(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("BaseAddress", &baseAddress_);
    registerAttribute("Length", &length_);
    registerAttribute("Tech", &tech_);
    registerAttribute("AdcDetector", &adc_detector_);

    baseAddress_.make_uint64(0);
    length_.make_uint64(0);
    tech_.make_uint64(0);
    adc_detector_.make_uint64(0);

    regs_ = DEFAULT_CONFIG;
}

PNP::~PNP() {
}

void PNP::postinitService() {
    regs_.tech.bits.tech = static_cast<uint8_t>(tech_.to_uint64());
    regs_.tech.bits.adc_detect =
        static_cast<uint8_t>(adc_detector_.to_uint64());
}

void PNP::b_transport(Axi4TransactionType *trans) {
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
}

}  // namespace debugger

