/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Operation interface implemented by slave devices on bus.
 */

#ifndef __DEBUGGER_IMEMOP_PLUGIN_H__
#define __DEBUGGER_IMEMOP_PLUGIN_H__

#include "iface.h"
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_MEMORY_OPERATION = "IMemoryOperation";

struct Axi4TransactionType {
    uint8_t rw;
    uint64_t addr;
    uint32_t rpayload[4];       // 128 bits data width
    uint32_t wstrb;             // 1 bit per byte
    uint32_t wpayload[4];
    uint8_t  xsize;             // [Bytes] Do not using XSize AXI format!!!.
};

class IMemoryOperation : public IFace {
public:
    IMemoryOperation() : IFace(IFACE_MEMORY_OPERATION) {}

    virtual void transaction(Axi4TransactionType *payload) =0;

    virtual uint64_t getBaseAddress() =0;

    virtual uint64_t getLength() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IMEMOP_PLUGIN_H__
