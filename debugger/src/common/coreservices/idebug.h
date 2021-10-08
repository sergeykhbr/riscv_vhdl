/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_DEBUG = "IDebug";

class IDebug : public IFace {
 public:
    IDebug() : IFace(IFACE_DEBUG) {}

    virtual int hartTotal() = 0;
    virtual void hartSelect(int hartidx) = 0;
    virtual int getHartSelected() = 0;
    virtual void reqResume(int hartidx) = 0;
    virtual void reqHalt(int hartidx) = 0;
    virtual bool isHalted(int hartidx) = 0;
    virtual void setResetPin(bool val) = 0;
};

}  // namespace debugger

