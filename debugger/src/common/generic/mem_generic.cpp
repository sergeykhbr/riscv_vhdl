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
#include "mem_generic.h"

namespace debugger {

MemoryGeneric::MemoryGeneric(const char *name)  : IService(name) {
    registerInterface(static_cast<IMemoryOperation *>(this));
    registerAttribute("ReadOnly", &readOnly_);
    registerAttribute("DpiClient", &dpiClient_);
    registerAttribute("DpiRoutes", &dpiRoutes_);

    readOnly_.make_boolean(false);
    mem_ = NULL;
    idpi_ = 0;
}

MemoryGeneric::~MemoryGeneric() {
    if (mem_) {
        delete mem_;
    }
}

void MemoryGeneric::postinitService() {
    mem_ = new uint8_t[static_cast<unsigned>(length_.to_uint64())];

    if (dpiClient_.is_string() && dpiClient_.size()) {
        idpi_ = static_cast<IDpi *>(
            RISCV_get_service_iface(dpiClient_.to_string(), IFACE_DPI));
        if (!idpi_) {
            RISCV_error("Can't get IDPi interface %s", dpiClient_.to_string());
        }
    }
}

ETransStatus MemoryGeneric::b_transport(Axi4TransactionType *trans) {
    uint64_t off = (trans->addr - getBaseAddress()) % length_.to_int();
    trans->response = MemResp_Valid;
    if (trans->action == MemAction_Write) {
        if (readOnly_.to_bool()) {
            RISCV_error("Write to READ ONLY memory", NULL);
            trans->response = MemResp_Error;
        } else if (((1ul << trans->xsize) - 1) == trans->wstrb) {
            memcpy(&mem_[off], trans->wpayload.b8, trans->xsize);
        } else {
            for (uint64_t i = 0; i < trans->xsize; i++) {
                if (((trans->wstrb >> i) & 0x1) == 0) {
                    continue;
                }
                mem_[off + i] = trans->wpayload.b8[i];
            }
        }

        /** Access to SystemVerilog */
        if (idpi_ && dpiRoutes_[trans->source_idx].to_bool()) {
            idpi_->axi4_write(off, static_cast<int>(trans->xsize),
                              trans->wpayload.b64[0]);
        }
    } else {
        trans->rpayload.b64[0] = 0;
        memcpy(trans->rpayload.b8, &mem_[off], trans->xsize);

        /** Access to SystemVerilog and auto-comparision */
        if (idpi_ && dpiRoutes_[trans->source_idx].to_bool()) {
            Reg64Type t1;
            idpi_->axi4_read(off, static_cast<int>(trans->xsize), &t1.val);

            if (t1.val != trans->rpayload.b64[0]) {
                RISCV_error("DPI diff [%08x]: %016" RV_PRI64 "x != %016" RV_PRI64 "x",
                    static_cast<unsigned>(off),
                    t1.val,
                    trans->rpayload.b64[0]
                    );
            }
        }
    }

    const char *rw_str[2] = {"=>", "<="};
    uint32_t *pdata[2] = {trans->rpayload.b32, trans->wpayload.b32};
    RISCV_debug("[%08" RV_PRI64 "x] %s [%08x %08x]",
        trans->addr,
        rw_str[trans->action],
        pdata[trans->action][1], pdata[trans->action][0]);
    return TRANS_OK;
}

}  // namespace debugger
