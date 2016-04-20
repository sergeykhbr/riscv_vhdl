/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Hap interface declaration.
 */

#ifndef __DEBUGGER_IHAP_H__
#define __DEBUGGER_IHAP_H__

#include "iface.h"
#include <stdarg.h>

namespace debugger {

static const char *const IFACE_HAP = "IHap";

enum EHapType {
    HAP_All,
    HAP_ConfigDone,
    HAP_BreakSimulation
};

class IHap : public IFace {
public:
    IHap(EHapType type = HAP_All) : IFace(IFACE_HAP), type_(type) {}

    EHapType getType() { return type_; }

    virtual void hapTriggered(EHapType type) =0;

protected:
    EHapType type_;
};

}  // namespace debugger

#endif  // __DEBUGGER_IHAP_H__
