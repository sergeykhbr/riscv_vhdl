/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Base interface declaration of the Core.
 */

#ifndef __DEBUGGER_IINSTRUCTION_H__
#define __DEBUGGER_IINSTRUCTION_H__

#include <inttypes.h>
#include <fstream>
#include "coreservices/ibus.h"

namespace debugger {

struct CpuContextType {
    uint64_t regs[32];
    uint64_t csr[1<<12];
    uint64_t pc;
    uint64_t npc;
    uint64_t exception;
    uint64_t interrupt;
    uint64_t step_cnt;
    uint64_t cur_prv_level;
    bool reset;
    IBus *ibus;
    char disasm[256];
    std::ofstream *reg_trace_file;
    std::ofstream *mem_trace_file;
};


static const char *const IFACE_INSTRUCTION = "IInstruction";

class IInstruction : public IFace {
public:
    IInstruction() : IFace(IFACE_INSTRUCTION) {}

    virtual const char *name() =0;
    virtual bool parse(uint32_t *payload) =0;
    virtual void exec(uint32_t *payload, CpuContextType *regs) =0;
    virtual uint32_t hash() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IINSTRUCTION_H__
