/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RISC-V simulating CPU interface.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
#define __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_CPU_RISCV = "ICpuRiscV";
static const char *const IFACE_DBG_NB_RESPONSE = "IDbgNbResponse";

static const uint64_t REG_INVALID   = ~0;
/** Signal types */
static const int CPU_SIGNAL_RESET   = 0;
static const int CPU_SIGNAL_EXT_IRQ = 1;

struct DebugPortTransactionType {
    bool write;
    uint8_t region;
    uint16_t addr;
    uint64_t wdata;
    uint64_t rdata;
};

class IDbgNbResponse : public IFace {
public:
    IDbgNbResponse() : IFace(IFACE_DBG_NB_RESPONSE) {}

    virtual void nb_response_debug_port(DebugPortTransactionType *trans) =0;
};


class ICpuRiscV : public IFace {
public:
    ICpuRiscV() : IFace(IFACE_CPU_RISCV) {}

    virtual void raiseSignal(int idx) =0;
    virtual void lowerSignal(int idx) =0;
    virtual void nb_transport_debug_port(DebugPortTransactionType *trans,
                                         IDbgNbResponse *cb) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_CPU_RISCV_H__
