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

#ifndef __DEBUGGER_SERVICES_MEM_MEMLUT_H__
#define __DEBUGGER_SERVICES_MEM_MEMLUT_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"

namespace debugger {

class MemoryLUT : public IService,
                  public IMemoryOperation {
 public:
    explicit MemoryLUT(const char *name);

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

 private:
    AttributeType memTarget_;
    AttributeType memOffset_;
    IMemoryOperation *itarget_;
};

DECLARE_CLASS(MemoryLUT)

}  // namespace debugger

#endif  // __DEBUGGER_SERVICES_MEM_MEMLUT_H__
