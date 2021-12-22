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
#include "ddr_ctrl.h"

namespace debugger {

DdrController::DdrController(const char *name) : RegMemBankGeneric(name),
    ctrl0(static_cast<IService *>(this), "ctrl0", 0x000),
    ctrl19(static_cast<IService *>(this), "ctrl19", 0x04C),
    ctrl21(static_cast<IService *>(this), "ctrl21", 0x054),
    ctrl120(static_cast<IService *>(this), "ctrl120", 0x1E0),
    ctrl132(static_cast<IService *>(this), "ctrl132", 0x210),
    ctrl136(static_cast<IService *>(this), "ctrl136", 0x220) {
}

void DdrController::postinitService() {
    RegMemBankGeneric::postinitService();
}


}  // namespace debugger

