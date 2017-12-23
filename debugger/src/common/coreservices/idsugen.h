/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Generic Debug Support Unit simulating interface.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_IDSUGEN_H__
#define __DEBUGGER_COMMON_CORESERVICES_IDSUGEN_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_DSU_GENERIC = "IDsuGeneric";

class IDsuGeneric : public IFace {
 public:
    IDsuGeneric() : IFace(IFACE_DSU_GENERIC) {}

    /** Bus utilization statistic methods */
    virtual void incrementRdAccess(int mst_id) = 0;
    virtual void incrementWrAccess(int mst_id) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICPUGEN_H__
