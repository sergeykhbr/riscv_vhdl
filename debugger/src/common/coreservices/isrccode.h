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

enum ESymbolType {
    SYMBOL_TYPE_FILE     = 0x01,
    SYMBOL_TYPE_FUNCTION = 0x02,
    SYMBOL_TYPE_DATA     = 0x04
};

enum ESymbolInfoListItem {
    Symbol_Name,
    Symbol_Addr,
    Symbol_Size,
    Symbol_Type,
    Symbol_Total
};


enum EBreakList {
    BrkList_address,
    BrkList_flags,
    BrkList_instr,
    BrkList_Total
};

enum EListTypes {
    AsmList_disasm,
    AsmList_symbol,
};

enum EColumnNames {
    ASM_list_type,
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

    /** Control Debug Symbols */
    virtual void addFileSymbol(const char *name, uint64_t addr, int sz) =0;

    virtual void addFunctionSymbol(const char *name, uint64_t addr, int sz) =0;

    virtual void addDataSymbol(const char *name, uint64_t addr, int sz) =0;

    virtual void addSymbols(AttributeType *list) =0;

    virtual void getSymbols(AttributeType *list) =0;

    virtual void addressToSymbol(uint64_t addr, AttributeType *info) =0;

    virtual int symbol2Address(const char *name, uint64_t *addr) =0;

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
     * @param[in] instr Original opcode before EBREAK instruction injection.
     */
    virtual void registerBreakpoint(uint64_t addr, uint64_t flags,
                                    uint64_t instr) =0;

    /** Unregister breakpoint at specified address.
     *
     * @param[in]  addr  Breakpoint location
     * @param[out] instr Original instruction value.
     * @param[out] flags Breakpoint flags.
     * @param[out] instr Original opcode rewriten by EBREAK instruction.
     * @return 0 if no errors
     */
    virtual int unregisterBreakpoint(uint64_t addr, uint64_t *flags,
                                    uint64_t *instr) =0;

    /** Get list of breakpoints.
     *
     * @param[out] lst Breakpoint list.
     */
    virtual void getBreakpointList(AttributeType *list) =0;

    /** Check specified address on breakpoint */
    virtual bool isBreakpoint(uint64_t addr, AttributeType *outbr) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ISRCCODE_H__
