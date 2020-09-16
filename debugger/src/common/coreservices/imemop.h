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

#ifndef __DEBUGGER_IMEMOP_PLUGIN_H__
#define __DEBUGGER_IMEMOP_PLUGIN_H__

#include <inttypes.h>
#include <iface.h>
#include <attribute.h>

namespace debugger {
class IService;

static const char *const IFACE_MEMORY_OPERATION = "IMemoryOperation";
static const char *const IFACE_AXI4_NB_RESPONSE = "IAxi4NbResponse";
static const char *const IFACE_ADDRESS_TRANSLATOR = "IAddressTranslator";

static const int PAYLOAD_MAX_BYTES = 8;

enum EAxi4Action {
    MemAction_Read,
    MemAction_Write,
    MemAction_Total
};

enum EAxi4Response {
    MemResp_Valid,
    MemResp_Accepted,
    MemResp_Error
};

enum ETransStatus {
    TRANS_OK,
    TRANS_ERROR
};

typedef struct Axi4TransactionType {
    EAxi4Action action;
    EAxi4Response response;
    uint32_t xsize;             // [Bytes] Isn't used XSize AXI format!!!.
    uint32_t wstrb;             // 1 bit per byte
    uint64_t addr;
    union {
        uint8_t b8[PAYLOAD_MAX_BYTES];
        uint16_t b16[PAYLOAD_MAX_BYTES/sizeof(uint16_t)];
        uint32_t b32[PAYLOAD_MAX_BYTES/sizeof(uint32_t)];
        uint64_t b64[PAYLOAD_MAX_BYTES/sizeof(uint64_t)];
    } rpayload, wpayload;
    int source_idx;             // Need for bus utilization statistic
} Axi4TransactionType;

/**
 * Non-blocking memory access response interface (Initiator/Master)
 */
class IAxi4NbResponse : public IFace {
 public:
    IAxi4NbResponse() : IFace(IFACE_AXI4_NB_RESPONSE) {}

    virtual void nb_response(Axi4TransactionType *trans) = 0;
};

/**
 * Slave/Targer interface
 */
class IMemoryOperation : public IFace {
 public:
    IMemoryOperation() : IFace(IFACE_MEMORY_OPERATION) {
        imap_.make_list(0);
        listMap_.make_list(0);
        baseAddress_.make_uint64(0);
        length_.make_uint64(0);
    }

    /** 
     * Add new device to memory space. Mapping device has to implement
     * IMemoryOperaton interface.
     */
    virtual void map(IMemoryOperation *imemop) {
        AttributeType t1(imemop);
        imap_.add_to_list(&t1);
    }

    /**
     * Blocking transaction
     *
     * Must be implemented by any functional/systemc device mapped into memory
     */
    virtual ETransStatus b_transport(Axi4TransactionType *trans) = 0;

    /**
     * Non-blocking transaction
     *
     * Can be implemented for interaction with the SystemC model for an example.
     * Default implementation re-direct to blocking transport
     */
    virtual ETransStatus nb_transport(Axi4TransactionType *trans,
                              IAxi4NbResponse *cb) {
        ETransStatus ret = b_transport(trans);
        cb->nb_response(trans);
        return ret;
    }

    virtual uint64_t getBaseAddress() { return baseAddress_.to_uint64(); }
    virtual void setBaseAddress(uint64_t addr) {
        baseAddress_.make_uint64(addr);
    }

    virtual uint64_t getLength() { return length_.to_uint64(); }
    virtual void setLength(uint64_t len) { return length_.make_uint64(len); }

    /** Higher value, higher priority */
    virtual int getPriority() { return priority_.to_int(); }
    virtual void setPriority(int v) { priority_.make_int64(v); }

 protected:
    friend class IService;
    AttributeType listMap_;
    AttributeType imap_;
    AttributeType baseAddress_;
    AttributeType length_;
    AttributeType priority_;
};

}  // namespace debugger

#endif  // __DEBUGGER_IMEMOP_PLUGIN_H__
