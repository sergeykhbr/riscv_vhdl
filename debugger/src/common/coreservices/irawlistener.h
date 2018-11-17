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

#ifndef __DEBUGGER_COMMON_CORESERVICES_IRAWLISTENER_H__
#define __DEBUGGER_COMMON_CORESERVICES_IRAWLISTENER_H__

#include <iface.h>

namespace debugger {

static const char *IFACE_RAW_LISTENER = "IRawListener";

class IRawListener : public IFace {
 public:
    IRawListener() : IFace(IFACE_RAW_LISTENER) {}

    virtual void updateData(const char *buf, int buflen) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IRAWLISTENER_H__
