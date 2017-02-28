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
#include "riscv-isa.h"
#include "coreservices/ibus.h"
#include "coreservices/isocinfo.h"

namespace debugger {

static const int STACK_TRACE_BUF_SIZE = 256;

struct CpuContextType {
    uint64_t regs[Reg_Total];
    uint64_t csr[1<<12];
    uint64_t pc;
    uint64_t npc;
    uint64_t exception;
    uint64_t interrupt;
    uint64_t interrupt_pending;
    uint64_t step_cnt;
    uint64_t cur_prv_level;
    DsuMapType::udbg_type::debug_region_type::breakpoint_control_reg br_ctrl;
    bool br_status_ena;     // show breakpoint bit in common status register
    bool br_inject_fetch;
    uint64_t br_address_fetch;
    uint32_t br_instr_fetch;
    bool reset;
    IBus *ibus;
    char disasm[256];
    std::ofstream *reg_trace_file;
    std::ofstream *mem_trace_file;
    uint64_t stack_trace_buf[STACK_TRACE_BUF_SIZE]; // [[from,to],*]
    int stack_trace_cnt;
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
