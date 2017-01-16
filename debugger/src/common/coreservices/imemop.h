/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Operation interface implemented by slave devices on bus.
 */

#ifndef __DEBUGGER_IMEMOP_PLUGIN_H__
#define __DEBUGGER_IMEMOP_PLUGIN_H__

#include "iface.h"
#include <inttypes.h>
#include "isocinfo.h"

namespace debugger {

static const char *const IFACE_MEMORY_OPERATION = "IMemoryOperation";
static const char *const IFACE_AXI4_NB_RESPONSE = "IAxi4NbResponse";

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

    virtual void nb_response(Axi4TransactionType *trans) =0;
};

/**
 * Slave/Targer interface
 */
class IMemoryOperation : public IFace {
public:
    IMemoryOperation() : IFace(IFACE_MEMORY_OPERATION) {}

    /**
     * Blocking transaction
     *
     * Must be implemented by any functional/systemc device mapped into memory
     */
    virtual void b_transport(Axi4TransactionType *trans) =0;

    /**
     * Non-blocking transaction
     *
     * Can be implemented for interaction with the SystemC model for an example.
     * Default implementation re-direct to blocking transport
     */
    virtual void nb_transport(Axi4TransactionType *trans,
                              IAxi4NbResponse *cb) {
        b_transport(trans);
        cb->nb_response(trans);
    }

    virtual uint64_t getBaseAddress() =0;

    virtual uint64_t getLength() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IMEMOP_PLUGIN_H__
