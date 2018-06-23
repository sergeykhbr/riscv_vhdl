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

#ifndef __DEBUGGER_SOCSIM_PLUGIN_UART_H__
#define __DEBUGGER_SOCSIM_PLUGIN_UART_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iserial.h"
#include "coreservices/iwire.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iclock.h"
#include <string>

namespace debugger {

class UART : public IService, 
             public IMemoryOperation,
             public ISerial {
 public:
    UART(const char *name);
    ~UART();

    /** IService interface */
    virtual void postinitService();

    /** IMemoryOperation */
    virtual ETransStatus b_transport(Axi4TransactionType *trans);

    /** ISerial */
    virtual int writeData(const char *buf, int sz);
    virtual void registerRawListener(IFace *listener);
    virtual void unregisterRawListener(IFace *listener);
    virtual void getListOfPorts(AttributeType *list);
    virtual int openPort(const char *port, AttributeType settings);
    virtual void closePort();

 private:
    AttributeType irqctrl_;
    AttributeType listeners_;  // non-registering attribute
    AttributeType autoTestEna_;
    AttributeType testCases_;
    IWire *iwire_;

    std::string input_;
    static const int RX_FIFO_SIZE = 16;
    char rxfifo_[RX_FIFO_SIZE];
    char *p_rx_wr_;
    char *p_rx_rd_;
    int rx_total_;
    mutex_def mutexListeners_;

    struct uart_map {
        volatile uint32_t status;
        volatile uint32_t scaler;
        uint32_t rsrv[2];
        volatile uint32_t data;
    } regs_;

  private:
    class AutoTest : public IClockListener {
     public:
        AutoTest(ISerial *parent, AttributeType *tests);

        /** IClockListener */
        virtual void stepCallback(uint64_t t);
     private:
        ISerial *parent_;
        AttributeType tests_;
        unsigned testcnt_;
    };
    AutoTest *pautotest_;
};

DECLARE_CLASS(UART)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_UART_H__
