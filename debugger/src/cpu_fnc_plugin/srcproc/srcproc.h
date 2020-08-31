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

#ifndef __DEBUGGER_SRCPROC_H__
#define __DEBUGGER_SRCPROC_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/isrccode.h"

namespace debugger {

typedef int (*disasm_opcode_f)(ISourceCode *isrc, uint64_t pc, uint32_t code,
                              AttributeType *mnemonic, AttributeType *comment);

typedef int (*disasm_opcode16_f)(ISourceCode *isrc, uint64_t pc,
                                Reg16Type code, AttributeType *mnemonic,
                                AttributeType *comment);

class RiscvSourceService : public IService,
                           public ISourceCode {
public:
    explicit RiscvSourceService(const char *name);
    virtual ~RiscvSourceService();

    /** IService interface */
    virtual void postinitService();

    /** ISourceCode interface */
    virtual void addFileSymbol(const char *name, uint64_t addr, int sz);

    virtual void addFunctionSymbol(const char *name, uint64_t addr, int sz);

    virtual void addDataSymbol(const char *name, uint64_t addr, int sz);

    virtual void addSymbols(AttributeType *list);

    virtual void clearSymbols();

    virtual void getSymbols(AttributeType *list) {
        *list = symbolListSortByName_;
    }

    virtual void addressToSymbol(uint64_t addr, AttributeType *info);

    virtual int symbol2Address(const char *name, uint64_t *addr);

    virtual int disasm(uint64_t pc,
                       uint8_t *data,
                       int offset,
                       AttributeType *mnemonic,
                       AttributeType *comment);
    virtual void disasm(uint64_t pc,
                       AttributeType *idata,
                       AttributeType *asmlist);

    virtual void registerBreakpoint(uint64_t addr, uint64_t flags,
                                    uint32_t instr, uint32_t opcode,
                                    uint32_t oplen);

    virtual int unregisterBreakpoint(uint64_t addr);

    virtual void getBreakpointList(AttributeType *list);

    virtual bool isBreakpoint(uint64_t addr);

private:
    disasm_opcode_f tblOpcode1_[32];
    disasm_opcode16_f tblCompressed_[32];
    AttributeType brList_;
    AttributeType symbolListSortByName_;
    AttributeType symbolListSortByAddr_;
};

DECLARE_CLASS(RiscvSourceService)

}  // namespace debugger

#endif  // __DEBUGGER_SRCPROC_H__
