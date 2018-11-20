/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Motor with sensors interface.
 */

#ifndef __DEBUGGER_PLUGIN_IMOTOR_H__
#define __DEBUGGER_PLUGIN_IMOTOR_H__

#include <iface.h>
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_MOTOR_LISTENER = "IMotorListener";

class IMotorListener : public IFace {
 public:
    IMotorListener() : IFace(IFACE_MOTOR_LISTENER) {}

    virtual void rotate(double deg) = 0;
};

static const char *const IFACE_MOTOR = "IMotor";

class IMotor : public IFace {
 public:
    IMotor() : IFace(IFACE_MOTOR) {}

    virtual void forwardRevolution() = 0;
    virtual void backwardRevolution() = 0;
    virtual void stopRevolution() = 0;

    /** 0 = motor is stopped; 1.0 = maximum rpm and enabled Breaks */
    virtual double getPowerConsumption() = 0;

    /** Backward pressure: 1.0 = 100% oclusion; 0 = no backward pressure */
    virtual double getForceResistance() = 0;
    virtual void changeForceResistance(double v) = 0;
    virtual double getForceMax() = 0;
    virtual void setForceForward(double N) = 0;
    virtual void setForceBackward(double N) = 0;

    virtual void registerMotorListener(IMotorListener *cb) = 0;
    virtual void unregisterMotorListener(IMotorListener *cb) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IMOTOR_H__
