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

#ifndef __DEBUGGER_ISERIAL_H__
#define __DEBUGGER_ISERIAL_H__

#include <iface.h>

namespace debugger {

static const char *IFACE_SERIAL = "ISerial";

class ISerial : public IFace {
 public:
    ISerial() : IFace(IFACE_SERIAL) {}

    /**
    * @brief Write data buffer from external module.
    * @return Number of written bytes.
    */
    virtual int writeData(const char *buf, int sz) = 0;

    virtual void registerRawListener(IFace *listener) = 0;
    virtual void unregisterRawListener(IFace *listener) = 0;

    virtual void getListOfPorts(AttributeType *list) = 0;
    virtual int openPort(const char *port, AttributeType settings) = 0;
    virtual void closePort() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ISERIAL_H__
