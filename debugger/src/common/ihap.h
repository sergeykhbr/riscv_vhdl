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

#ifndef __DEBUGGER_IHAP_H__
#define __DEBUGGER_IHAP_H__

#include <iface.h>
#include <stdarg.h>

namespace debugger {

static const char *const IFACE_HAP = "IHap";

enum EHapType {
    HAP_All,
    HAP_ConfigDone,
    HAP_Halt,
    HAP_BreakSimulation,
    HAP_CpuTurnON,
    HAP_CpuTurnOFF
};

class IHap : public IFace {
 public:
    explicit IHap(EHapType type = HAP_All) : IFace(IFACE_HAP), type_(type) {}

    EHapType getType() { return type_; }

    virtual void hapTriggered(IFace *isrc, EHapType type,
                             const char *descr) = 0;

 protected:
    EHapType type_;
};

}  // namespace debugger

#endif  // __DEBUGGER_IHAP_H__
