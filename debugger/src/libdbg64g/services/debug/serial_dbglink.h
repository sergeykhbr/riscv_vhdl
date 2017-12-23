/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debuger transport level via Serial link implementation.
 * @details
 *             Write command: 
 *                 Send     [11.Length-1].Addr[63:0].Data[31:0]*(x Length)
 *             Read command: 
 *                 Send     [10.Length-1].Addr[63:0]
 *                 Receive  Data[31:0]*(x Length)
 */

#ifndef __DEBUGGER_SERIAL_DBGLINK_SERVICE_H__
#define __DEBUGGER_SERIAL_DBGLINK_SERVICE_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/itap.h"
#include "coreservices/iserial.h"
#include "coreservices/irawlistener.h"

namespace debugger {

static const int UART_REQ_HEADER_SZ = 9;
static const int UART_MST_BURST_MAX = 64;

#pragma pack(1)
struct UartMstPacketType {
    uint8_t cmd;
    uint64_t addr;
    uint32_t data[UART_MST_BURST_MAX];
};
#pragma pack()

union PacketType {
    UartMstPacketType fields;
    char buf[1];
};

class SerialDbgService : public IService,
                         public ITap,
                         public IRawListener {
public:
    SerialDbgService(const char *name);
    ~SerialDbgService();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService();

    /** ITap interface */
    virtual int read(uint64_t addr, int bytes, uint8_t *obuf);
    virtual int write(uint64_t addr, int bytes, uint8_t *ibuf);

    /** IRawListener interface */
    virtual void updateData(const char *buf, int buflen);

private:
    AttributeType timeout_;
    AttributeType port_;

    ISerial *iserial_;
    event_def event_block_;
    PacketType pkt_;
    int rd_count_;
    int req_count_;
};

DECLARE_CLASS(SerialDbgService)

}  // namespace debugger

#endif  // __DEBUGGER_SERIAL_DBGLINK_SERVICE_H__
