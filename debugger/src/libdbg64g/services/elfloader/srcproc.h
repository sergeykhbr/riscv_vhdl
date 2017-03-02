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
#include "coreservices/ielfreader.h"

namespace debugger {

typedef int (*disasm_opcode_f)(IElfReader *ielf,
                                uint64_t pc,
                                uint32_t code,
                                AttributeType *mnemonic,
                                AttributeType *comment);

class SourceService : public IService,
                      public ISourceCode {
public:
    explicit SourceService(const char *name);
    virtual ~SourceService();

    /** IService interface */
    virtual void postinitService();

    /** ISourceCode interface */
    virtual int disasm(uint64_t pc,
                       uint8_t *data,
                       int offset,
                       AttributeType *mnemonic,
                       AttributeType *comment);
    virtual void disasm(uint64_t pc,
                       AttributeType *idata,
                       AttributeType *asmlist);

    virtual void registerBreakpoint(uint64_t addr, uint32_t instr,
                                    uint64_t flags);

    virtual int unregisterBreakpoint(uint64_t addr, uint32_t *instr,
                                     uint64_t *flags);

    virtual void getBreakpointList(AttributeType *list);


private:
    disasm_opcode_f tblOpcode1_[32];
    AttributeType brList_;
    IElfReader *ielf_;
};

DECLARE_CLASS(SourceService)

}  // namespace debugger

#endif  // __DEBUGGER_SRCPROC_H__
