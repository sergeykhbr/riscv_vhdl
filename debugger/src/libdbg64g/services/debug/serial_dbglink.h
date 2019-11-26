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

#ifndef __DEBUGGER_SERIAL_DBGLINK_SERVICE_H__
#define __DEBUGGER_SERIAL_DBGLINK_SERVICE_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/itap.h"
#include "coreservices/iserial.h"
#include "coreservices/irawlistener.h"

namespace debugger {

#define MAGIC_ID 0x31
static const int UART_REQ_HEADER_SZ = 10;
static const int UART_MST_BURST_WORD_MAX = 64;
static const int UART_MST_BURST_BYTES_MAX = 4*UART_MST_BURST_WORD_MAX;

#pragma pack(1)
struct UartMstPacketType {
    uint8_t magic;
    uint8_t cmd;
    uint64_t addr;
    uint8_t data8[UART_MST_BURST_BYTES_MAX];
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
    virtual int updateData(const char *buf, int buflen);

private:
    AttributeType timeout_;
    AttributeType port_;

    ISerial *iserial_;
    event_def event_block_;
    PacketType pkt_;
    int rd_count_;
    int req_count_;
    int wait_bytes_;
    uint8_t rx_buf_[UART_MST_BURST_BYTES_MAX + 16];
};

DECLARE_CLASS(SerialDbgService)

}  // namespace debugger

#endif  // __DEBUGGER_SERIAL_DBGLINK_SERVICE_H__
