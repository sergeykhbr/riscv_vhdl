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

#ifndef __DEBUGGER_COMMON_CORESERVICES_IAUTOCOMPLETE_H__
#define __DEBUGGER_COMMON_CORESERVICES_IAUTOCOMPLETE_H__

#include <iface.h>
#include <attribute.h>

namespace debugger {

static const char *IFACE_AUTO_COMPLETE = "IAutoComplete";

/** Qt Compatible Virtual Keys */
#if defined(_WIN32) || defined(__CYGWIN__)
static const uint32_t KB_Backspace = 0x0008;
static const uint32_t KB_Tab = 0x0009;
static const uint32_t KB_Return = 0x000d;
static const uint32_t KB_Shift = 0x0010;
static const uint32_t KB_Control = 0x0011;
static const uint32_t KB_Alt = 0x0012;
static const uint32_t KB_Escape = 0x001b;
static const uint32_t KB_PageUp = 0x0021;
static const uint32_t KB_PageDown = 0x0022;
static const uint32_t KB_End = 0x0023;
static const uint32_t KB_Home = 0x0024;
static const uint32_t KB_Left = 0x0025;
static const uint32_t KB_Up = 0x0026;
static const uint32_t KB_Right = 0x0027;
static const uint32_t KB_Down = 0x0028;
static const uint32_t KB_Delete = 0x002e;
static const uint32_t KB_Dot = 0x00be;
#else
static const uint32_t KB_Backspace = 0xff08;
static const uint32_t KB_Tab = 0xff09;
static const uint32_t KB_Return = 0xff0d;
static const uint32_t KB_Shift = 0xffe1;
static const uint32_t KB_Control = 0xffe3;
static const uint32_t KB_Alt = 0xffe9;
static const uint32_t KB_Escape = 0xff1b;
static const uint32_t KB_PageUp = 0xff25;
static const uint32_t KB_PageDown = 0xffe4;
//  static const uint32_t KB_End = 0x0000;
//  static const uint32_t KB_Home = 0x0000;
static const uint32_t KB_Left = 0xff23;
static const uint32_t KB_Up = 0xff26;
static const uint32_t KB_Right = 0xff22;
static const uint32_t KB_Down = 0xff8d;
static const uint32_t KB_Delete = 0xff61;
static const uint32_t KB_Dot = 0x002e;
#endif


class IAutoComplete : public IFace {
 public:
    IAutoComplete() : IFace(IFACE_AUTO_COMPLETE) {}

    /**
     * @return New command ready flag
     */
    virtual bool processKey(uint32_t qt_key, AttributeType *cmd,
                            AttributeType *cursor) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IAUTOCOMPLETE_H__
