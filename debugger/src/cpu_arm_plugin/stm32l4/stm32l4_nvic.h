/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_SRC_STM32L4XX_STM32L4_NVIC_H__
#define __DEBUGGER_SRC_STM32L4XX_STM32L4_NVIC_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class STM32L4_NVIC : public IService {
 public:
    explicit STM32L4_NVIC(const char *name);

    /** IService interface */
    virtual void postinitService() override;

 protected:
    MappedReg32Type NVIC_ISER0;     // interrupt 0 to 31
    MappedReg32Type NVIC_ISER1;     // interrupt 32 to 63
    MappedReg32Type NVIC_ISER2;
    MappedReg32Type NVIC_ISER3;
    MappedReg32Type NVIC_ISER4;
    MappedReg32Type NVIC_ISER5;     // interrupt 260 to 191
    MappedReg32Type NVIC_ISER6;     // interrupt 192 to 223
    MappedReg32Type NVIC_ISER7;     // interrupt 224 to 239
};

DECLARE_CLASS(STM32L4_NVIC)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_STM32L4XX_STM32L4_NVIC_H__
