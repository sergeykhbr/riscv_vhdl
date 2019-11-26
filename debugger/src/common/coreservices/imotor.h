/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
