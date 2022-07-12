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

 #pragma once

#include <api_types.h>
#include <iface.h>
#include <attribute.h>
#include "coreservices/itap.h"
#include "coreservices/imemop.h"

namespace debugger {

static const char *IFACE_COMMAND = "ICommand";

static const int CMD_VALID      = 0;
static const int CMD_WRONG_ARGS = 1;
static const int CMD_INVALID    = 2;

const uint32_t REG_ADDR_ERROR = 0xFFFFFFFFul;

class ICommand : public IFace {
 public:
    ICommand(const char *name, uint64_t dmibar, ITap *tap)
        : IFace(IFACE_COMMAND), tap_(tap), ibus_(0), dmibar_(dmibar) {
        cmdName_.make_string(name);
    }

    virtual const char *cmdName() { return cmdName_.to_string(); }
    virtual const char *briefDescr() { return briefDescr_.to_string(); }
    virtual const char *detailedDescr() { return detailedDescr_.to_string(); }
    // Enabling access from command to bus transactions:
    virtual void enableDMA(IMemoryOperation *ibus, uint64_t dmibar) {
        ibus_ = ibus;
        dmibar_ = dmibar;
    }
    virtual int isValid(AttributeType *args) = 0;
    virtual void exec(AttributeType *args, AttributeType *res) = 0;

    virtual void generateError(AttributeType *res, const char *descr) {
        res->make_list(3);
        (*res)[0u].make_string("ERROR");
        (*res)[1].make_string(cmdName_.to_string());
        (*res)[2].make_string(descr);
    }

 protected:
    // Non-blocking access to the selected bus from the command execution thread
    virtual ETransStatus dma_read(uint64_t addr, uint32_t sz, uint8_t *payload) {
        ETransStatus ret = TRANS_ERROR;
        if (!ibus_) {
            return ret;
        }
        Axi4TransactionType tr;
        tr.action = MemAction_Read;
        tr.addr = addr;
        uint32_t bytecnt = 0;
        while (bytecnt < sz) {
            if ((sz - bytecnt) > 8) {
                tr.xsize = 8;
            } else {
                tr.xsize = sz - bytecnt;
            }
            nbobj_.clear();
            ret = ibus_->nb_transport(&tr,
                                      static_cast<IAxi4NbResponse *>(&nbobj_));
            nbobj_.wait();
            memcpy(payload, nbobj_.rpayload(), tr.xsize);
            if (ret != TRANS_OK) {
                break;
            }
            bytecnt += tr.xsize;
            payload += tr.xsize;
            tr.addr += tr.xsize;
        }
        return ret;
    }
    virtual ETransStatus dma_write(uint64_t addr, uint32_t sz, uint8_t *payload) {
        ETransStatus ret = TRANS_ERROR;
        if (!ibus_) {
            return ret;
        }
        Axi4TransactionType tr;
        tr.action = MemAction_Write;
        tr.addr = addr;
        uint32_t bytecnt = 0;
        while (bytecnt < sz) {
            if ((sz - bytecnt) > 8) {
                tr.xsize = 8;
            } else {
                tr.xsize = sz - bytecnt;
            }
            tr.wstrb = (1u << tr.xsize) - 1;
            memcpy(tr.wpayload.b8, payload, tr.xsize);
            nbobj_.clear();
            ret = ibus_->nb_transport(&tr,
                                      static_cast<IAxi4NbResponse *>(&nbobj_));
            nbobj_.wait();
            if (ret != TRANS_OK) {
                break;
            }
            bytecnt += tr.xsize;
            payload += tr.xsize;
            tr.addr += tr.xsize;
        }
        return ret;
    }

 protected:
    AttributeType cmdName_;
    AttributeType briefDescr_;
    AttributeType detailedDescr_;
    ITap *tap_;
    IMemoryOperation *ibus_;
    uint64_t dmibar_; 

    // We should use nb-access to provide access to SystemC or we will
    // have deadlock on system bus access (from cpu and waiting response on DMI)
    class NbLockObject : public IAxi4NbResponse {
     public:
        NbLockObject() : IAxi4NbResponse() {
            AttributeType t1;
            RISCV_generate_name(&t1);
            RISCV_event_create(&event_nb_, t1.to_string());
        }
        virtual ~NbLockObject() {
            RISCV_event_close(&event_nb_);
        }

        /** IAxi4NbResponse */    
        virtual void nb_response(Axi4TransactionType *trans) {
            resp_ = *trans;
            RISCV_event_set(&event_nb_);
        }

        // Common methods
        void clear() {
            RISCV_event_clear(&event_nb_);
        }
        void wait() {
            RISCV_event_wait(&event_nb_);
        }
        uint8_t *rpayload() {
            return resp_.rpayload.b8;
        }
     private:
        event_def event_nb_;
        Axi4TransactionType resp_;
    } nbobj_;
};

}  // namespace debugger
