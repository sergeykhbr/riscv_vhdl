/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Sensor's interface.
 */

#ifndef __DEBUGGER_PLUGIN_ISENSOR_H__
#define __DEBUGGER_PLUGIN_ISENSOR_H__

#include <iface.h>
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_SENSOR = "ISensor";

class ISensor : public IFace {
 public:
    ISensor() : IFace(IFACE_SENSOR) {}

    virtual void changeSensorValue(double rate) = 0;
    virtual double getSensorValue() = 0;
    virtual double getPhysicalValue() {
        return getSensorValue();
    }
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ISENSOR_H__
