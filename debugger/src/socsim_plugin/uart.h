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
#include "coreservices/icommand.h"
#include "coreservices/icmdexec.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class UartCmdType : public ICommand {
 public:
    UartCmdType(IService *parent, const char *name) : ICommand(name, 0) {
        parent_ = parent;
        iserial_ = static_cast<ISerial *>(parent->getInterface(IFACE_SERIAL));
        briefDescr_.make_string("Access to external Serial port from console.");
        detailedDescr_.make_string(
            "Read/Write value:\n"
            "    serial_objname 'string value' to transmit\n"
            "Response:\n"
            "    data or string\n"
            "Usage:\n"
            "    uart0 'help'");
    }

    /** ICommand */
    virtual int isValid(AttributeType *args) {
        if (!iserial_ || !(*args)[0u].is_equal(parent_->getObjName())) {
            return CMD_INVALID;
        }
        return CMD_VALID;
    }

    virtual void exec(AttributeType *args, AttributeType *res) {
        res->make_nil();
        if (args->size() > 1 && (*args)[1].is_string()) {
            char eol[3] = "\r\n";
            iserial_->writeData((*args)[1].to_string(), (*args)[1].size());
            iserial_->writeData(eol, 2);
        }
    }

 private:
    IService *parent_;
    ISerial *iserial_;
};

class UART : public RegMemBankGeneric,
             public ISerial,
             public IClockListener {
 public:
    explicit UART(const char *name);
    virtual ~UART();

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** ISerial */
    virtual int writeData(const char *buf, int sz);
    virtual void registerRawListener(IFace *listener);
    virtual void unregisterRawListener(IFace *listener);
    virtual void getListOfPorts(AttributeType *list);
    virtual int openPort(const char *port, AttributeType settings);
    virtual void closePort();

    /** IClockListener */
    virtual void stepCallback(uint64_t t);

    /** Common methods */
    void setScaler(uint32_t scaler);
    int getFifoSize() { return fifoSize_.to_int(); }
    int getRxTotal() { return rx_total_; }
    int getTxTotal() { return tx_total_; }
    void putByte(char v);
    char getByte();
    uint64_t getExecCounter() { return iclk_->getExecCounter(); }

 protected:
    class STATUS_TYPE : public MappedReg64Type {
     public:
        STATUS_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg64Type(parent, name, addr, 4) {
            value_type t;
            t.v = 0;
            t.b.rx_irq_ena = 1;
            t.b.tx_irq_ena = 1;
            value_.val = t.v;
        }

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

     protected:
        virtual uint64_t aboutToWrite(uint64_t new_val) override;
    };

    class DWORD_TYPE : public MappedReg64Type {
     public:
        DWORD_TYPE(IService *parent, const char *name, uint64_t addr) :
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
    AttributeType clock_;
    AttributeType cmdexec_;
    AttributeType listeners_;  // non-registering attribute

    IWire *iwire_;
    ICmdExecutor *icmdexec_;
    IClock *iclk_;

    char *rxfifo_;
    char *p_rx_wr_;
    char *p_rx_rd_;
    int rx_total_;

    static const int FIFOSZ = 15;
    char tx_fifo_[FIFOSZ];
    int tx_wcnt_;
    int tx_rcnt_;
    int tx_total_;

    mutex_def mutexListeners_;
    UartCmdType *pcmd_;

    STATUS_TYPE status_;
    SCALER_TYPE scaler_;
    DWORD_TYPE fwcpuid_;
    DATA_TYPE data_;
    int t_cb_cnt_;
};

DECLARE_CLASS(UART)

}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_PLUGIN_UART_H__
