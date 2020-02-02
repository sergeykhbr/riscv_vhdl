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

#include "api_core.h"
#include "rmembank_gen1.h"

namespace debugger {

RegMemBankGeneric::RegMemBankGeneric(const char *name)
    : IService(name), IHap(HAP_ConfigDone) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    stubmem = 0;
    imaphash_ = 0;

    RISCV_register_hap(static_cast<IHap *>(this));
}

RegMemBankGeneric::~RegMemBankGeneric() {
    if (stubmem) {
        delete [] stubmem;
    }
    if (imaphash_) {
        delete [] imaphash_;
    }
}

void RegMemBankGeneric::postinitService() {
    imaphash_ = new IMemoryOperation *[length_.to_int()];
    memset(imaphash_, 0, length_.to_int()*sizeof(IMemoryOperation *));

    IMemoryOperation *imem;
    for (unsigned i = 0; i < listMap_.size(); i++) {
        const AttributeType &dev = listMap_[i];
        if (dev.is_string()) {
            imem = static_cast<IMemoryOperation *>(RISCV_get_service_iface(
                    dev.to_string(), IFACE_MEMORY_OPERATION));
            if (imem == 0) {
                RISCV_error("Can't find mapped reg %s", dev.to_string());
                continue;
            }
            map(imem);
        } else if (dev.is_list() && dev.size() == 2) {
            const AttributeType &devname = dev[0u];
            const AttributeType &portname = dev[1];
            imem = static_cast<IMemoryOperation *>(
                RISCV_get_service_port_iface(devname.to_string(),
                                             portname.to_string(),
                                             IFACE_MEMORY_OPERATION));
            if (imem == 0) {
                RISCV_error("Can't find mapped reg %s:%s", 
                    devname.to_string(), portname.to_string());
                continue;
            }
            map(imem);
        }
    }
    stubmem = new uint8_t[length_.to_int()];
    memset(stubmem, 0xFF, length_.to_int());
}

/** We need correctly mapped device list to compute hash, postinit
    doesn't allow to guarantee order of initialization. */
void RegMemBankGeneric::hapTriggered(IFace *isrc,
                                 EHapType type,
                                 const char *descr) {
    IMemoryOperation *imem;
    for (unsigned i = 0; i < imap_.size(); i++) {
        imem = static_cast<IMemoryOperation *>(imap_[i].to_iface());
        maphash(imem);
    }
}

ETransStatus RegMemBankGeneric::b_transport(Axi4TransactionType *trans) {
    IMemoryOperation *imem;
    uint64_t t_addr = trans->addr;      // orignal address

    trans->addr -= getBaseAddress();    // offset relative registers bank
    imem = imaphash_[trans->addr];
    if (imem != 0) {
        ETransStatus ret = imem->b_transport(trans);
        trans->addr = t_addr;           // restore address;
        return ret;
    }
    
    // Stubs:
    uint64_t off = trans->addr;
    if (trans->action == MemAction_Read) {
        trans->rpayload.b64[0] = 0;
        memcpy(trans->rpayload.b8, &stubmem[off], trans->xsize);
        RISCV_info("Read stub  [%08" RV_PRI64 "x] => 0x%" RV_PRI64 "x",
                    t_addr, trans->rpayload.b64[0]);
    } else {
        memcpy(&stubmem[off], trans->wpayload.b8, trans->xsize);
        uint64_t msk = (0x1ull << 8*trans->xsize) - 1;
        RISCV_info("Write stub [%08" RV_PRI64 "x] <= 0x%" RV_PRI64 "x",
                    t_addr, trans->wpayload.b64[0] & msk);
    }
    trans->addr = t_addr;           // restore address;
    return TRANS_OK;
}

ETransStatus RegMemBankGeneric::nb_transport(Axi4TransactionType *trans,
                                             IAxi4NbResponse *cb) {
    IMemoryOperation *imem;
    uint64_t t_addr = trans->addr;      // orignal address
    
    trans->addr -= getBaseAddress();    // offset relative registers bank
    imem = imaphash_[trans->addr];
    if (imem != 0) {
        ETransStatus ret = imem->nb_transport(trans, cb);
        trans->addr = t_addr;           // restore address;
        return ret;
    }
    
    trans->addr = t_addr;               // restore address;
    ETransStatus ret = b_transport(trans);
    cb->nb_response(trans);
    return ret;
}


void RegMemBankGeneric::maphash(IMemoryOperation *imemop) {
    // All Registers inside bank mapped relative register bank baseAddress
    uint64_t off = imemop->getBaseAddress();
    if (!imaphash_) {
        return;
    }
    if (off >= length_.to_uint64()) {
        RISCV_printf(0, 0,
                "Map out-of-range %08" RV_PRI64 "x => %08" RV_PRI64 "x",
                imemop->getBaseAddress(),
                length_.to_uint64());
        return;
    }
    for (unsigned i = 0; i < imemop->getLength(); i++) {
        if (imaphash_[off + i] && 
            imaphash_[off + i]->getPriority() > imemop->getPriority()) {
            continue;
        }
        if (imaphash_[off + i]) {
            RISCV_printf(0, 0, "[0,'%s','overmap register 0x%04x']",
                        obj_name_.to_string(), off + i);
        }
        imaphash_[off + i] = imemop;
    }
}

IMemoryOperation *RegMemBankGeneric::getRegFace(uint64_t addr) {
    uint64_t off = addr - getBaseAddress();
    return imaphash_[off];
}

}  // namespace debugger

