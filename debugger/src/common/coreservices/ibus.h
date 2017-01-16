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

struct BusUtilType {
    uint64_t w_cnt;
    uint64_t r_cnt;
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

    /**
     * This method emulates connection between bus controller and DSU module.
     * It allows to read bus utilization statistic via mapped DSU registers.
     */
    virtual BusUtilType *bus_utilization() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IBUS_PLUGIN_H__
