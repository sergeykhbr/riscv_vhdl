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

#ifndef __DEBUGGER_COMMON_CORESERVICES_ICLOCK_H__
#define __DEBUGGER_COMMON_CORESERVICES_ICLOCK_H__

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>

namespace debugger {

static const char *const IFACE_CLOCK_LISTENER = "IClockListener";

class IClockListener : public IFace {
 public:
    IClockListener() : IFace(IFACE_CLOCK_LISTENER) {}

    virtual void stepCallback(uint64_t t) = 0;
};


static const char *const IFACE_CLOCK = "IClock";

class IClock : public IFace {
 public:
    IClock() : IFace(IFACE_CLOCK) {}

    virtual uint64_t getStepCounter() = 0;

    /** Executed instruction counter.
     *
     * One executed instruction = 1 step for functional simulation.
     * And it can be more than 1 for precise SystemC model if enabled
     * GENERATE_CORE_TRACE.
     */
    virtual void registerStepCallback(IClockListener *cb, uint64_t t) = 0;

    /** Move already registered callback to another time */
    virtual bool moveStepCallback(IClockListener *cb, uint64_t t) = 0;

    virtual double getFreqHz() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICLOCK_H__
