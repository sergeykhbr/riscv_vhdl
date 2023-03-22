/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_PLUGIN_ISRCCODE_H__
#define __DEBUGGER_PLUGIN_ISRCCODE_H__

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>

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
    BrkList_flags,  // breakpoint type
    BrkList_instr,  // original instruction opcode
    BrkList_opcode, // sw breakpoint instruction opcode
    BrkList_oplen,  // breakpoint opcode length
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
    virtual void addFileSymbol(const char *name, uint64_t addr, int sz) = 0;

    virtual void addFunctionSymbol(const char *name, uint64_t addr,
                                   int sz) = 0;

    virtual void addDataSymbol(const char *name, uint64_t addr, int sz) = 0;

    virtual void clearSymbols() = 0;
    virtual void addSymbols(AttributeType *list) = 0;

    virtual void getSymbols(AttributeType *list) = 0;

    virtual void addressToSymbol(uint64_t addr, AttributeType *info) = 0;

    virtual int symbol2Address(const char *name, uint64_t *addr) = 0;

    /** Disasm input data buffer.
     *
     * @return disassembled instruction length
     */
    virtual void disasm(int mode,
                       uint64_t pc,
                       AttributeType *idata,
                       AttributeType *asmlist) = 0;


    /** Add breakpoint at specified address.
     *
     * @param[in] addr  Breakpoint location
     * @param[in] flags Hardware Breakpoint and others if needed
     */
    virtual void addBreakpoint(uint64_t addr, uint64_t flags) = 0;

    /** Remove breakpoint at specified address.
     *
     * @param[in]  addr  Breakpoint location
     * @return 0 if no errors
     */
    virtual int removeBreakpoint(uint64_t addr) = 0;

    /** Get list of breakpoints.
     *
     * @param[out] lst Breakpoint list.
     */
    virtual void getBreakpointList(AttributeType *list) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ISRCCODE_H__
