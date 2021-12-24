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
#include "coreservices/ispi.h"
#include <iostream>

namespace debugger {

class SdCard : public IService, 
               public IMemoryOperation,
               public ISlaveSPI {
 public:
    explicit SdCard(const char *name);
    virtual ~SdCard();

    /** IService interface */
    virtual void postinitService() override;

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** ISlaveSPI */
    virtual size_t spiWrite(uint64_t addr, uint8_t *buf, size_t bufsz);
    virtual size_t spiRead(uint64_t addr, uint8_t *buf, size_t bufsz);

 protected:
    AttributeType image_;

    FILE *file_;
};

DECLARE_CLASS(SdCard)

}  // namespace debugger

