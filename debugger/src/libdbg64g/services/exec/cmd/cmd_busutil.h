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
 *
 * @details    Read CPU dport registers: clock counter and per
 *             master counters with read/write transactions to compute
 *             utilization characteristic.
 */

#ifndef __DEBUGGER_CMD_BUSUTIL_H__
#define __DEBUGGER_CMD_BUSUTIL_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/icommand.h"
#include "debug/dsumap.h"

namespace debugger {

class CmdBusUtil : public ICommand  {
 public:
    explicit CmdBusUtil(ITap *tap);

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

 private:
    uint64_t clock_cnt_z_;
    DsuMapType::local_regs_type::local_region_type::mst_bus_util_type
    bus_util_z_[32];
    uint32_t mst_total_;
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_BUSUTIL_H__
