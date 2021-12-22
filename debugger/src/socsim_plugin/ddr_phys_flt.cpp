/**
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

#include "api_core.h"
#include "ddr_phys_flt.h"

namespace debugger {

DdrPhysFilter::DdrPhysFilter(const char *name) : RegMemBankGeneric(name),
    devicepmp0(static_cast<IService *>(this), "devicepmp0", 0x00),
    devicepmp1(static_cast<IService *>(this), "devicepmp1", 0x08),
    devicepmp2(static_cast<IService *>(this), "devicepmp2", 0x10),
    devicepmp3(static_cast<IService *>(this), "devicepmp3", 0x18) {
    registerAttribute("DdrController", &ddrctrl_);
}

void DdrPhysFilter::postinitService() {
    RegMemBankGeneric::postinitService();
}


uint64_t DdrPhysFilter::PMP_TYPE::aboutToWrite(uint64_t nxt_val) {
    DdrPhysFilter *p = static_cast<DdrPhysFilter *>(parent_);
    return nxt_val;
}

}  // namespace debugger

