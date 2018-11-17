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

#ifndef __DEBUGGER_ELFREADER_H__
#define __DEBUGGER_ELFREADER_H__

#include <stdint.h>
#include <iface.h>
#include <attribute.h>

namespace debugger {

static const char *const IFACE_ELFREADER = "IElfReader";

class IElfReader : public IFace {
 public:
    IElfReader() : IFace(IFACE_ELFREADER) {}

    virtual int readFile(const char *filename) = 0;

    virtual unsigned loadableSectionTotal() = 0;

    virtual const char *sectionName(unsigned idx) = 0;

    virtual uint64_t sectionAddress(unsigned idx) = 0;

    virtual uint64_t sectionSize(unsigned idx) = 0;

    virtual uint8_t *sectionData(unsigned idx) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ELFREADER_H__
