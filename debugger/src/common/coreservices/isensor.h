/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_PLUGIN_ISENSOR_H__
#define __DEBUGGER_PLUGIN_ISENSOR_H__

#include <iface.h>
#include <inttypes.h>
#include "coreservices/icmdexec.h"

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
    virtual void setPhysicalValue(double rate) {
        changeSensorValue(rate);
    }
};

class SensorCmdType : public ICommand {
 public:
    SensorCmdType(IService *parent, const char *name)
        : ICommand(parent, name) {
        isen_ = static_cast<ISensor *>(parent->getInterface(IFACE_SENSOR));
        briefDescr_.make_string("Generic Sensor instance management command.");
        detailedDescr_.make_string(
            "Description:\n"
            "    Read Sensor's value or change it\n"
            "Read/Write value:\n"
            "    sensor_objname <write_value>\n"
            "Response:\n"
            "    double f1:\n"
            "        f1  - sensor value\n"
            "Usage:\n"
            "    sensor0\n"
            "    sensor1 7.5");
    }

    /** ICommand */
    virtual int isValid(AttributeType *args) {
        if (!isen_ || !(*args)[0u].is_equal(cmdParent_->getObjName())) {
            return CMD_INVALID;
        }
        return CMD_VALID;
    }

    virtual void exec(AttributeType *args, AttributeType *res) {
        double vol = isen_->getPhysicalValue();
        if (args->size() > 1) {
            if ((*args)[1].is_integer()) {
                vol = (*args)[1].to_int();
            } else if ((*args)[1].is_floating()) {
                vol = (*args)[1].to_float();
            } else if ((*args)[1].is_bool()) {
                vol = (*args)[1].to_bool() ? 1.0 : 0.0;
            }
            isen_->setPhysicalValue(vol);
        }
        res->make_floating(vol);
    }

 protected:
    ISensor *isen_;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_ISENSOR_H__
