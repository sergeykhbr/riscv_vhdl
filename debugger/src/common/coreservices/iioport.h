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

#ifndef __DEBUGGER_COMMON_CORESERVICES_IIOPORT_H__
#define __DEBUGGER_COMMON_CORESERVICES_IIOPORT_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *IFACE_IOPORT = "IIOPort";

class IIOPort : public IFace {
 public:
    IIOPort() : IFace(IFACE_IOPORT) {}

    virtual void registerPortListener(IFace *listener) = 0;
    virtual void unregisterPortListener(IFace *listener) = 0;
};

static const char *IFACE_IOPORT_LISTENER8 = "IIOPortListener8";

class IIOPortListener8 : public IFace {
 public:
    IIOPortListener8() : IFace(IFACE_IOPORT_LISTENER8) {}

    virtual void readData(uint8_t *val, uint8_t mask) = 0;
    virtual void writeData(uint8_t val, uint8_t mask) = 0;
    virtual void latch() = 0;
    virtual void pullup(uint8_t, uint8_t mask) {}
};

static const char *IFACE_IOPORT_LISTENER16 = "IIOPortListener16";

class IIOPortListener16 : public IFace {
public:
    IIOPortListener16() : IFace(IFACE_IOPORT_LISTENER16) {}

    virtual void readData(uint16_t *val, uint16_t mask) = 0;
    virtual void writeData(uint16_t val, uint16_t mask) = 0;
    virtual void latch() = 0;
};

static const char *IFACE_IOPORT_LISTENER32 = "IIOPortListener32";

class IIOPortListener32 : public IFace {
public:
    IIOPortListener32() : IFace(IFACE_IOPORT_LISTENER32) {}

    virtual void readData(uint32_t *val, uint32_t mask) = 0;
    virtual void writeData(uint32_t val, uint32_t mask) = 0;
    virtual void latch() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IIOPORT_H__
