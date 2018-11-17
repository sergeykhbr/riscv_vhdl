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

#ifndef __DEBUGGER_COMMON_CORESERVICES_IDSUGEN_H__
#define __DEBUGGER_COMMON_CORESERVICES_IDSUGEN_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_DSU_GENERIC = "IDsuGeneric";

class IDsuGeneric : public IFace {
 public:
    IDsuGeneric() : IFace(IFACE_DSU_GENERIC) {}

    /** Bus utilization statistic methods */
    virtual void incrementRdAccess(int mst_id) = 0;
    virtual void incrementWrAccess(int mst_id) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ICPUGEN_H__
