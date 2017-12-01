/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Source code processor class declaration.
 */

#ifndef __DEBUGGER_SRCPROC_H__
#define __DEBUGGER_SRCPROC_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/isrccode.h"

namespace debugger {

typedef int (*disasm_opcode_f)(ISourceCode *isrc, uint64_t pc, uint32_t code,
                              AttributeType *mnemonic, AttributeType *comment);

class SourceService : public IService,
                      public ISourceCode {
public:
    explicit SourceService(const char *name);
    virtual ~SourceService();

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
                                    uint64_t instr);

    virtual int unregisterBreakpoint(uint64_t addr, uint64_t *flags,
                                    uint64_t *instr);

    virtual void getBreakpointList(AttributeType *list);

    virtual bool isBreakpoint(uint64_t addr, AttributeType *outbr);

private:
    disasm_opcode_f tblOpcode1_[32];
    AttributeType brList_;
    AttributeType symbolListSortByName_;
    AttributeType symbolListSortByAddr_;
};

DECLARE_CLASS(SourceService)

}  // namespace debugger

#endif  // __DEBUGGER_SRCPROC_H__
