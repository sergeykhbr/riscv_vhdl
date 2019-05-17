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

#ifndef __SRC_COMMON_GENERIC_BUS_GENERIC_H__
#define __SRC_COMMON_GENERIC_BUS_GENERIC_H__

#include <iclass.h>
#include <iservice.h>
#include <ihap.h>
#include "coreservices/imemop.h"
#include "generic/mapreg.h"

namespace debugger {

class BusGeneric : public IService,
                   public IMemoryOperation,
                   public IHap {
 public:
    explicit BusGeneric(const char *name);
    virtual ~BusGeneric();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation interface */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                                      IAxi4NbResponse *cb);

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

 protected:
    /** Speed-optimized mapping */
    virtual uint64_t adr_hash(uint64_t adr) { return adr; }
    virtual void maphash(IMemoryOperation *imemop) {}
    virtual IMemoryOperation *getHashedDevice(uint64_t addr) { return 0; }
    void getMapedDevice(Axi4TransactionType *trans,
                        IMemoryOperation **pdev, uint32_t *sz);

 protected:
    AttributeType defaultSlave_;
    AttributeType useHash_;
    mutex_def mutexBAccess_;
    mutex_def mutexNBAccess_;
    Axi4TransactionType b_tr_;
    Axi4TransactionType nb_tr_;

    GenericReg64Bank busUtil_;    // per master read/write access statistic
    IMemoryOperation **imaphash_;
    IMemoryOperation *idefmem_;
};

DECLARE_CLASS(BusGeneric)

}  // namespace debugger

#endif  // __SRC_COMMON_GENERIC_BUS_GENERIC_H__
