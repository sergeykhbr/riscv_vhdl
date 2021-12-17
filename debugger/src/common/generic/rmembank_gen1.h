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

#pragma once

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "coreservices/imemop.h"

namespace debugger {

class RegMemBankGeneric : public IService, 
                          public IMemoryOperation,
                          public IHap {
 public:
    explicit RegMemBankGeneric(const char *name);
    virtual ~RegMemBankGeneric();

    /** IService interface */
    virtual void postinitService() override;

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                              IAxi4NbResponse *cb);


    /** IHap */
    virtual void hapTriggered(EHapType type, uint64_t param,
                              const char *descr);

 protected:
    /** Speed-optimized mapping */
    void maphash(IMemoryOperation *imemop);
    IMemoryOperation *getRegFace(uint64_t addr);
    
 protected:
    IMemoryOperation **imaphash_;
    uint8_t *stubmem;
};

}  // namespace debugger
