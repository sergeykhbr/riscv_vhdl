/**
 * @file
 * @copyright  Copyright 2018 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Code Coverage tracker interface.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_ICOVERAGETRACKER_H__
#define __DEBUGGER_COMMON_CORESERVICES_ICOVERAGETRACKER_H__

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>
#include <iservice.h>
#include "coreservices/icmdexec.h"

namespace debugger {

static const char *const IFACE_COVERAGE_TRACKER = "ICoverageTracker";

class ICoverageTracker : public IFace {
 public:
    ICoverageTracker() : IFace(IFACE_COVERAGE_TRACKER) {}

    virtual void markAddress(uint64_t addr, uint8_t oplen) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICOVERAGETRACKER_H__
