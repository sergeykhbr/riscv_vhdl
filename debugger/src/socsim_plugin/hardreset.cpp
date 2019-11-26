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

#include <api_core.h>
#include "hardreset.h"

namespace debugger {

HardReset::HardReset(const char *name)  : IService(name) {
}

void HardReset::postinitService() {
}

void HardReset::powerOnPressed() {
    IResetListener *irst;
    AttributeType rstlist;
    RISCV_get_iface_list(IFACE_RESET_LISTENER, &rstlist);
    for (unsigned i = 0; i < rstlist.size(); i++) {
        irst = static_cast<IResetListener *>(rstlist[i].to_iface());
        irst->reset(static_cast<IService *>(this));
    }

    IPower *ipwr;
    AttributeType pwrlist;
    RISCV_get_iface_list(IFACE_POWER, &pwrlist);
    for (unsigned i = 0; i < pwrlist.size(); i++) {
        ipwr = static_cast<IPower *>(pwrlist[i].to_iface());
        ipwr->power(POWER_OFF);
    }

    RISCV_info("%d devices were resetted. VCC OFF",
            rstlist.size());
}

void HardReset::powerOnReleased() {
}

}  // namespace debugger

