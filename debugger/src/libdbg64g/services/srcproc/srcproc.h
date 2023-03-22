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
#include <ihap.h>
#include "coreservices/isrccode.h"
#include "coreservices/icmdexec.h"
#include "cmd_br.h"

namespace debugger {

class RiscvSourceService : public IService,
                           public IHap,
                           public ISourceCode {
public:
    explicit RiscvSourceService(const char *name);
    virtual ~RiscvSourceService();

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** IHap interface */
    virtual void hapTriggered(EHapType type,
                              uint64_t param,
                              const char *descr);


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

    virtual void disasm(int mode,
                        uint64_t pc,
                        AttributeType *idata,
                        AttributeType *asmlist);

    virtual void addBreakpoint(uint64_t addr, uint64_t flags);

    virtual int removeBreakpoint(uint64_t addr);

    virtual void getBreakpointList(AttributeType *list);

    virtual bool isBreakpoint(uint64_t addr);

private:
    AttributeType cmdexec_;

    ICmdExecutor *icmdexec_;

    CmdBrRiscv *pcmdBr_;

    AttributeType brList_;
    AttributeType symbolListSortByName_;
    AttributeType symbolListSortByAddr_;
};

DECLARE_CLASS(RiscvSourceService)

}  // namespace debugger

#endif  // __DEBUGGER_SRCPROC_H__
