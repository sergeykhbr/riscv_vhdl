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
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class UART : public RegMemBankGeneric,
             public ISerial {
 public:
    explicit UART(const char *name);
    virtual ~UART();

    /** IService interface */
    virtual void postinitService();

    /** ISerial */
    virtual int writeData(const char *buf, int sz);
    virtual void registerRawListener(IFace *listener);
    virtual void unregisterRawListener(IFace *listener);
    virtual void getListOfPorts(AttributeType *list);
    virtual int openPort(const char *port, AttributeType settings);
    virtual void closePort();

    /** Common methods */
    int getFifoSize() { return fifoSize_.to_int(); }
    int getRxTotal() { return rx_total_; }
    int getTxTotal() { return 0; }
    void putByte(char v);
    char getByte();

 protected:
    class STATUS_TYPE : public MappedReg64Type {
     public:
        STATUS_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr, 4) {}

        union value_type {
            uint64_t v;
            struct bits_type {
                uint64_t tx_fifo_full : 1;      // [0]
                uint64_t tx_fifo_empty : 1;     // [1]
                uint64_t rsrv1 : 2;             // [3:2]
                uint64_t rx_fifo_full : 1;      // [4]
                uint64_t rx_fifo_empty : 1;     // [5]
                uint64_t rsrv2 : 2;             // [7:6]
                uint64_t err_parity : 1;        // [8]
                uint64_t err_stopbit : 1;       // [9]
                uint64_t rsrv3 : 3;             // [12:10]
                uint64_t rx_irq_ena : 1;        // [13]
                uint64_t tx_irq_ena : 1;        // [14]
                uint64_t parity_bit : 1;        // [15]
            } b;
        };

        STATUS_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
     protected:
        virtual uint64_t aboutToRead(uint64_t cur_val) override;
    };

    class SCALER_TYPE : public MappedReg64Type {
     public:
        SCALER_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr, 4) {}
    };

    class DATA_TYPE : public MappedReg64Type {
     public:
        DATA_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr, 4) {}
     protected:
        virtual uint64_t aboutToRead(uint64_t cur_val) override;
        virtual uint64_t aboutToWrite(uint64_t new_val) override;
    };

 private:
    AttributeType fifoSize_;
    AttributeType irqctrl_;
    AttributeType listeners_;  // non-registering attribute
    IWire *iwire_;

    char *rxfifo_;
    char *p_rx_wr_;
    char *p_rx_rd_;
    int rx_total_;
    mutex_def mutexListeners_;

    STATUS_TYPE status_;
    SCALER_TYPE scaler_;
    DATA_TYPE data_;
};

DECLARE_CLASS(UART)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_UART_H__
