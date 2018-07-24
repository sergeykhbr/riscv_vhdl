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

#include "soc_info.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(SocInfo)

SocInfo::SocInfo(const char *name) 
    : IService(name) {
    registerInterface(static_cast<ISocInfo *>(this));
    registerAttribute("PnpBaseAddress", &pnpBase_);
    registerAttribute("DsuBaseAddress", &dsuBase_);
    registerAttribute("GpioBaseAddress", &gpioBase_);
    registerAttribute("ListCSR", &listCSR_);
    registerAttribute("ListRegs", &listRegs_);

    dsuBase_.make_uint64(0);
    listCSR_.make_list(0);
    listRegs_.make_list(0);
}

void SocInfo::postinitService() {
}

unsigned SocInfo::getMastersTotal() {
    // todo: via reading PNP configuration
    return CFG_NASTI_MASTER_TOTAL;
}

unsigned SocInfo::getSlavesTotal() {
    // todo: via reading PNP configuration
    return 0;
}

unsigned SocInfo::getRegsTotal() {
    return listRegs_.size();
}

void SocInfo::getRegsList(AttributeType *lst) {
    lst->make_list(listRegs_.size());
    for (unsigned i = 0; i < listRegs_.size(); i++) {
        const AttributeType &name = listRegs_[i][0u];
        if (name.size() == 0) {
            continue;
        }
        (*lst)[i] = name;
    }
}

unsigned SocInfo::getCsrTotal() {
    return listCSR_.size();
}

void SocInfo::getCsrList(AttributeType *lst) {
    lst->make_list(listCSR_.size());
    for (unsigned i = 0; i < listCSR_.size(); i++) {
        (*lst)[i] = listCSR_[i][0u];
    }
}

uint64_t SocInfo::csr2addr(const char *name) {
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    AttributeType id(name);
    for (unsigned i = 0; i < listCSR_.size(); i++) {
        if (strcmp(id.to_upper(), listCSR_[i][0u].to_string()) == 0) {
            return reinterpret_cast<uint64_t>(
                &dsu->csr[listCSR_[i][2].to_uint64()]);
        }
    }
    return REG_ADDR_ERROR;
}

uint64_t SocInfo::reg2addr(const char *name) {
    DsuMapType *dsu = reinterpret_cast<DsuMapType *>(dsuBase_.to_uint64());
    for (unsigned i = 0; i < listRegs_.size(); i++) {
        if (strcmp(name, listRegs_[i][0u].to_string()) == 0) {
            return dsuBase_.to_uint64() + listRegs_[i][2].to_uint64();
        }
    }
    return REG_ADDR_ERROR;
}

uint64_t SocInfo::addressPlugAndPlay() {
    return pnpBase_.to_uint64();
}

uint64_t SocInfo::addressGpio() {
    return gpioBase_.to_uint64();
}

}  // namespace debugger
