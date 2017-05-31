/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug UART port model.
 */

#ifndef __DEBUGGER_SOCSIM_PLUGIN_UARTMST_H__
#define __DEBUGGER_SOCSIM_PLUGIN_UARTMST_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/ithread.h"
#include "coreservices/imemop.h"
#include "coreservices/iserial.h"
#include "coreservices/iwire.h"
#include "coreservices/irawlistener.h"
#include "coreservices/ibus.h"
#include <string>

namespace debugger {

static const int UART_MST_BURST_MAX = 64;

#pragma pack(1)
struct UartMstPacketType {
    uint8_t cmd;
    Reg64Type addr;
    Reg64Type data[UART_MST_BURST_MAX];
};
#pragma pack()

class UartMst : public IService, 
                public IThread,
                public IAxi4NbResponse,
                public ISerial {
public:
    UartMst(const char *name);
    ~UartMst();

    /** IService interface */
    virtual void postinitService();

    /** ISerial */
    virtual int writeData(const char *buf, int sz);
    virtual void registerRawListener(IFace *listener);
    virtual void unregisterRawListener(IFace *listener);

    /** IAxi4NbResponse */
    virtual void nb_response(Axi4TransactionType *trans);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    AttributeType listeners_;  // non-registering attribute
    AttributeType bus_;

    IBus *ibus_;
    ISerial *itransport_;

    Axi4TransactionType trans_;

    mutex_def mutexListeners_;
    event_def event_request_;
    event_def event_accept_;
    event_def event_tap_;

    union TxBufferType {
        UartMstPacketType packet;
        uint8_t buf[sizeof(UartMstPacketType)];
    } txbuf_;
    int txbuf_sz_;
    bool baudrate_detect_;

    struct uart_map {
        volatile uint32_t status;
        volatile uint32_t scaler;
        uint32_t rsrv[2];
        volatile uint32_t data;
    } regs_;
};

DECLARE_CLASS(UartMst)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_UARTMST_H__
