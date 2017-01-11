/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Source code interface: disassembler.
 */

#ifndef __DEBUGGER_PLUGIN_ISRCCODE_H__
#define __DEBUGGER_PLUGIN_ISRCCODE_H__

#include "iface.h"
#include <inttypes.h>
#include "attribute.h"

namespace debugger {

static const char *const IFACE_SOURCE_CODE = "ISourceCode";

enum EBreakList {
    BrkList_address,
    BrkList_instr,
    BrkList_hwflag,
    BrkList_Total
};

enum EColumnNames {
    ASM_addrline,
    ASM_code,
    ASM_codesize,
    ASM_breakpoint,
    ASM_label,
    ASM_mnemonic,
    ASM_comment,
    ASM_Total
};

static const uint64_t BreakFlag_HW = (1 << 0);

class ISourceCode : public IFace {
public:
    ISourceCode() : IFace(IFACE_SOURCE_CODE) {}

    /** Disasm input data buffer.
     *
     * @return disassembled instruction length
     */
    virtual int disasm(uint64_t pc,
                       uint8_t *data,
                       int offset,
                       AttributeType *mnemonic,
                       AttributeType *comment) =0;
    virtual void disasm(uint64_t pc,
                       AttributeType *idata,
                       AttributeType *asmlist) =0;


    /** Register breakpoint at specified address.
     *
     * @param[in] addr  Breakpoint location
     * @param[in] instr Current instruction value at specified address.
     *                  For HW breakpoint may have any value so that memory
     *                  won't be modified.
     * @param[in] hw    Breakpoint flags
     */
    virtual void registerBreakpoint(uint64_t addr, uint32_t instr,
                                    uint64_t flags) =0;

    /** Unregister breakpoint at specified address.
     *
     * @param[in]  addr  Breakpoint location
     * @param[out] instr Original instruction value.
     * @param[out] flags Breakpoint flags.
     * @return 0 if no errors
     */
    virtual int unregisterBreakpoint(uint64_t addr, uint32_t *instr,
                                     uint64_t *flags) =0;

    /** Get list of breakpoints.
     *
     * @param[out] lst Breakpoint list.
     */
    virtual void getBreakpointList(AttributeType *list) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ISRCCODE_H__
