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

#pragma once

#include <iclass.h>
#include <iservice.h>
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class DdrController : public RegMemBankGeneric {
 public:
    explicit DdrController(const char *name);

    /** IService interface */
    virtual void postinitService() override;

 private:
    MappedReg32Type ctrl0;          // [0x000] Control reg 0
    MappedReg32Type ctrl19;         // [0x04C] Control reg 19
    MappedReg32Type ctrl21;         // [0x054] Control reg 21
    MappedReg32Type ctrl120;        // [0x1E0] Control reg 120
    MappedReg32Type ctrl132;        // [0x210] Control reg 132
    MappedReg32Type ctrl136;        // [0x220] Control reg 136

};

DECLARE_CLASS(DdrController)

}  // namespace debugger
