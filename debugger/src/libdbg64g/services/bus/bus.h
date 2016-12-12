/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System Bus class declaration (AMBA or whatever).
 */

#ifndef __DEBUGGER_BUS_H__
#define __DEBUGGER_BUS_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/iclock.h"
#include "coreservices/ibus.h"
#include <string>

namespace debugger {

class Bus : public IService,
            public IBus {
public:
    explicit Bus(const char *name);
    virtual ~Bus();

    /** IService interface */
    virtual void postinitService();

    /** IBus interface */
    virtual void map(IMemoryOperation *imemop);
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                      IAxi4NbResponse *cb);
    virtual void addBreakpoint(uint64_t addr);
    virtual void removeBreakpoint(uint64_t addr);

private:
    AttributeType listMap_;
    AttributeType imap_;
    AttributeType breakpoints_;
    // Clock interface is used just to tag debug output with some step value,
    // in a case of several clocks the first found will be used.
    IClock *iclk0_;
    mutex_def mutexBAccess_;
    mutex_def mutexNBAccess_;
};

DECLARE_CLASS(Bus)

}  // namespace debugger

#endif  // __DEBUGGER_BUS_H__
