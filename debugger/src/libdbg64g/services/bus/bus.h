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
#include "coreservices/imemop.h"
#include "coreservices/isocinfo.h"
#include <string>

namespace debugger {

class Bus : public IService,
            public IMemoryOperation {
public:
    explicit Bus(const char *name);
    virtual ~Bus();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation interface */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                      IAxi4NbResponse *cb);
    virtual BusUtilType *bus_utilization();

private:
    // Clock interface is used just to tag debug output with some step value,
    // in a case of several clocks the first found will be used.
    IClock *iclk0_;
    mutex_def mutexBAccess_;
    mutex_def mutexNBAccess_;

    BusUtilType info_[CFG_NASTI_MASTER_TOTAL];
};

DECLARE_CLASS(Bus)

}  // namespace debugger

#endif  // __DEBUGGER_BUS_H__
