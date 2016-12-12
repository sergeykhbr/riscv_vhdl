/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System with Bus Direct Memory Access interface.
 * @details    Blocking and non-blocking transactions are supported.
 */

#ifndef __DEBUGGER_IBUS_PLUGIN_H__
#define __DEBUGGER_IBUS_PLUGIN_H__

#include "iface.h"
#include <inttypes.h>
#include "imemop.h"

namespace debugger {

static const char *const IFACE_BUS = "IBus";

enum ETransStatus {
    TRANS_OK,
    TRANS_ERROR
};

/**
 * Bus interface
 */
class IBus : public IFace {
public:
    IBus() : IFace(IFACE_BUS) {}

    virtual void map(IMemoryOperation *imemop) =0;

    /**
     * Blocking transaction. It is used for functional modeling of devices.
     */
    virtual ETransStatus b_transport(Axi4TransactionType *trans) =0;
    
    /**
     * Non-blocking transaction. Is is used to interract with SystemC models.
     * Usage with a functional models is also possible.
     */
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                      IAxi4NbResponse *cb) =0;

    virtual void addBreakpoint(uint64_t addr) =0;

    virtual void removeBreakpoint(uint64_t addr) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IBUS_PLUGIN_H__
