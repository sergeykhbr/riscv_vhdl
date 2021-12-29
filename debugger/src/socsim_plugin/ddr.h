/*
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

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"

namespace debugger {

class DDR : public IService, 
            public IMemoryOperation {
 public:
    explicit DDR(const char *name);
    virtual ~DDR();

    /** IService interface */
    virtual void postinitService() override;

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

 private:
    virtual uint8_t *getpMem(uint64_t addr);

 protected:
    static const int BLOCK_SIZE = 1024*1024;

    struct MemBlockType {
        MemBlockType *nxt;
        MemBlockType *prv;
        uint64_t bid;
        uint8_t m[BLOCK_SIZE];
    } mem_;
};

DECLARE_CLASS(DDR)

}  // namespace debugger

