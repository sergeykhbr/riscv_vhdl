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

#ifndef __DEBUGGER_SRC_CPU_FNC_PLUGIN_ICACHE_FUNC_H__
#define __DEBUGGER_SRC_CPU_FNC_PLUGIN_ICACHE_FUNC_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/imemop.h"
#include "coreservices/icmdexec.h"

namespace debugger {

class ICacheFunctional : public IService,
                         public IMemoryOperation,
                         public ICommand {
 public:
    explicit ICacheFunctional(const char *name);
    virtual ~ICacheFunctional();

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

 private:
    void runTest();

 private:
    AttributeType sysBus_;
    AttributeType cmdexec_;
    AttributeType totalKBytes_;  // default 64 KB
    AttributeType lineBytes_;    // default 32 B
    AttributeType ways_;         // default 4

    IMemoryOperation *isysbus_;
    ICmdExecutor *icmdexec_;

    /** By default suppose using:
            16 memory banks with 16 bits width to provide C-instruction
            alignment.
     */
    uint16_t **banks_;
};

DECLARE_CLASS(ICacheFunctional)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_FNC_PLUGIN_ICACHE_FUNC_H__
