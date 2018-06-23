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

#ifndef __DEBUGGER_SOCSIM_PLUGIN_UARTMST_H__
#define __DEBUGGER_SOCSIM_PLUGIN_UARTMST_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/ithread.h"
#include "coreservices/imemop.h"
#include "coreservices/iserial.h"
#include "coreservices/iwire.h"
#include "coreservices/irawlistener.h"
#include "coreservices/imemop.h"
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
    virtual void getListOfPorts(AttributeType *list);
    virtual int openPort(const char *port, AttributeType settings);
    virtual void closePort();

    /** IAxi4NbResponse */
    virtual void nb_response(Axi4TransactionType *trans);

protected:
    /** IThread interface */
    virtual void busyLoop();

private:
    AttributeType listeners_;  // non-registering attribute
    AttributeType bus_;

    IMemoryOperation *ibus_;
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
