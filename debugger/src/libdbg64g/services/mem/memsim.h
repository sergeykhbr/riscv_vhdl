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

#ifndef __DEBUGGER_SERVICES_MEM_MEMSIM_H__
#define __DEBUGGER_SERVICES_MEM_MEMSIM_H__

#include "generic/mem_generic.h"

namespace debugger {

class MemorySim : public MemoryGeneric {
 public:
    explicit MemorySim(const char *name);

    /** IService interface */
    virtual void postinitService() override;

 private:
    static const int SYMB_IN_LINE = 16/2;
    bool chishex(int s);
    uint8_t chtohex(int s);
    int readHexFile(const char *filename, uint8_t *buf, int bufsz);
    int readBinFile(const char *filename, uint8_t *buf, int bufsz);

 private:
    AttributeType initFile_;
    AttributeType binaryFile_;
};

DECLARE_CLASS(MemorySim)

}  // namespace debugger

#endif  // __DEBUGGER_SERVICES_MEM_MEMSIM_H__
