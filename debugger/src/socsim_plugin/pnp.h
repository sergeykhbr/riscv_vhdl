/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plug'n'Play device functional model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_PNP_H__
#define __DEBUGGER_SOCSIM_PLUGIN_PNP_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "types_amba.h"

namespace debugger {

typedef struct PnpConfigType {
    uint32_t xmask;
    uint32_t xaddr;
    uint16_t did;
    uint16_t vid;
    uint8_t size;
    uint8_t rsrv[3];
} PnpConfigType;

struct pnp_map {
    uint32_t hwid;         /// RO: HW ID
    uint32_t fwid;         /// RW: FW ID
    uint32_t tech;         /// RO: technology index
    uint32_t rsrv1;        /// 
    uint64_t idt;          /// 
    uint64_t malloc_addr;  /// RW: debuggind memalloc pointer 0x18
    uint64_t malloc_size;  /// RW: debugging memalloc size 0x20
    uint64_t fwdbg1;       /// RW: FW debug register
    uint64_t rsrv[2];
    PnpConfigType slaves[256];  // RO: slaves config
};


class PNP : public IService, 
            public IMemoryOperation {
public:
    PNP(const char *name);
    ~PNP();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual void transaction(Axi4TransactionType *payload);
    
    virtual uint64_t getBaseAddress() {
        return baseAddress_.to_uint64();
    }
    virtual uint64_t getLength() {
        return length_.to_uint64();
    }

private:
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType tech_;
    AttributeType adc_detector_;

    pnp_map regs_;
};

DECLARE_CLASS(PNP)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_PNP_H__
