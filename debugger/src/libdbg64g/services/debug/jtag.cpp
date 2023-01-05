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
#include "jtag.h"

namespace debugger {

JTAG::JTAG(const char *name) : IService(name) {
    registerInterface(static_cast<IJtag *>(this));
    registerAttribute("Target", &target_);
    ibitbang_ = 0;
}

JTAG::~JTAG() {
}

void JTAG::postinitService() {
    if (target_.is_list() && target_.size() == 2) {
        ibitbang_ = static_cast<IJtagTap *>(
            RISCV_get_service_port_iface(target_[0u].to_string(),
                                         target_[1].to_string(),
                                         IFACE_JTAG_TAP));
    }
    if (ibitbang_ == 0) {
        RISCV_error("Cannot get IJtagTap interface");
    }
}

void JTAG::TestReset() {
}

uint64_t JTAG::IR(uint64_t iscan, int sz) {
    return 0;
}

uint64_t JTAG::DR(uint64_t dscan, int sz) {
    return 0;
}


}  // namespace debugger
