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

#ifndef __DEBUGGER_COMMON_CORESERVICES_IKEYBOARD_H__
#define __DEBUGGER_COMMON_CORESERVICES_IKEYBOARD_H__

#include <iface.h>
#include <inttypes.h>

namespace debugger {

static const char *const IFACE_KEYBOARD = "IKeyboard";

class IKeyboard : public IFace {
 public:
    IKeyboard() : IFace(IFACE_KEYBOARD) {}

    virtual void keyPress(const char *keyname) = 0;
    virtual void keyRelease(const char *keyname) = 0;
    virtual int getRow() { return 0; }
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IKEYBOARD_H__
