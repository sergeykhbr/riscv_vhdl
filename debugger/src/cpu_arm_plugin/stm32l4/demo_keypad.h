/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_SRC_STM32L4XX_STM32L4_DEMO_KEYPAD_H__
#define __DEBUGGER_SRC_STM32L4XX_STM32L4_DEMO_KEYPAD_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ireset.h"
#include "coreservices/ikeyboard.h"
#include "coreservices/icmdexec.h"
#include "coreservices/iioport.h"
#include "generic/iotypes.h"
#include "generic/key_gen1.h"

namespace debugger {

class DemoKeypad : public IService,
                   public IKeyboard,
                   public IIOPortListener32 {
 public:
    explicit DemoKeypad(const char *name);
    virtual ~DemoKeypad();

    /** IService */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** IIOPortListener32 */
    virtual void readData(uint32_t *val, uint32_t mask);
    virtual void writeData(uint32_t val, uint32_t mask);
    virtual void latch() {}

    /** IKeyboard */
    virtual void keyPress(const char *keyname) {}
    virtual void keyRelease(const char *keyname) {}
    virtual int getRow() override;

 protected:
    AttributeType port_;
    AttributeType keys_;
    AttributeType cmdexec_;

    IIOPort *iport_;
    ICmdExecutor *iexec_;

    KeyGeneric32 **BTN;
    uint32_t col_;
};

DECLARE_CLASS(DemoKeypad)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_STM32L4XX_STM32L4_DEMO_KEYPAD_H__
