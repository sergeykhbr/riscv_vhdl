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

#include "api_core.h"
#include "sdcard.h"

namespace debugger {

SdCard::SdCard(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerInterface(static_cast<ISlaveSPI *>(this));

    registerAttribute("Image", &image_);

    file_ = NULL;
}

SdCard::~SdCard() {
    if (file_) {
        fclose(file_);
    }
}

void SdCard::postinitService() {
    file_ = fopen(image_.to_string(), "rb");
    if (!file_) {
        RISCV_error("Can't open file %s", image_.to_string());
    }
}

ETransStatus SdCard::b_transport(Axi4TransactionType *trans) {
    uint64_t off = (trans->addr - getBaseAddress()) % length_.to_int();
    trans->response = MemResp_Valid;

    RISCV_error("%s", "Sd-card memory access not implemented");
    return TRANS_OK;
}

size_t SdCard::spiWrite(uint64_t addr, uint8_t *buf, size_t bufsz) {
    return bufsz;
}

size_t SdCard::spiRead(uint64_t addr, uint8_t *buf, size_t bufsz) {
    if (!file_) {
        return 0;
    }

    if (ftell(file_) != addr) {
        fseek(file_, addr, SEEK_SET);
    }
    size_t ret = fread(buf, 1, bufsz, file_);
    return ret;
}

}  // namespace debugger
