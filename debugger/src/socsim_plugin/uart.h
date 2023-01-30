/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "iclass.h"
#include "iservice.h"
#include "coreservices/imemop.h"
#include "coreservices/iserial.h"
#include "coreservices/iirq.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iclock.h"
#include "coreservices/icommand.h"
#include "coreservices/icmdexec.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class UartCmdType : public ICommand {
 public:
    UartCmdType(IService *parent, uint64_t dmibar, const char *name)
        : ICommand(parent, name) {
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
        if (!iserial_ || !(*args)[0u].is_equal(cmdParent_->getObjName())) {
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
    uint32_t getScaler();
    int getFifoSize() { return fifoSize_.to_int(); }
    uint32_t getRxTotal() { return rx_total_; }
    uint32_t getTxTotal() { return tx_total_; }
    uint32_t getTxWatermark() { return txctrl_.getTyped().b.txcnt; }
    uint32_t getRxWatermark() { return rxctrl_.getTyped().b.rxcnt; }
    void putByte(char v);
    char getByte();

 protected:
    class TXCTRL_TYPE : public MappedReg32Type {
     public:
        TXCTRL_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t txen : 1;      // [0] TX enable
                uint32_t nstop : 1;     // [1] Number of stop bits
                uint32_t rsrv1 : 14;    // [15:2]
                uint32_t txcnt : 3;     // [18:16] tx watermark fifo
                uint32_t rsrv2 : 13;    // [31:19]
            } b;
        };

        TXCTRL_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
    };

    class RXCTRL_TYPE : public MappedReg32Type {
     public:
        RXCTRL_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t rxen : 1;      // [0] RX enable
                uint32_t rsrv1 : 15;    // [15:1]
                uint32_t rxcnt : 3;     // [18:16] rx watermark fifo
                uint32_t rsrv2 : 13;    // [31:19]
            } b;
        };

        RXCTRL_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
    };

    class SCALER_TYPE : public MappedReg32Type {
     public:
        SCALER_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

     protected:
        virtual uint32_t aboutToWrite(uint32_t new_val) override;
    };

    class FWCPUID_TYPE : public MappedReg32Type {
     public:
        FWCPUID_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}
        virtual uint32_t aboutToWrite(uint32_t new_val) override;
    };

    class TXDATA_TYPE : public MappedReg32Type {
     public:
        TXDATA_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}
     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
        virtual uint32_t aboutToWrite(uint32_t new_val) override;
    };

    class RXDATA_TYPE : public MappedReg32Type {
     public:
        RXDATA_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}
     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
    };

    class IEDATA_TYPE : public MappedReg32Type {
     public:
        IEDATA_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t txwm : 1;      // [0] 
                uint32_t rxwm : 1;      // [1]
                uint32_t rsrv : 30;     // [31:2]
            } b;
        };

        IEDATA_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
    };

    class IPDATA_TYPE : public MappedReg32Type {
     public:
        IPDATA_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t txwm : 1;      // [0] 
                uint32_t rxwm : 1;      // [1]
                uint32_t rsrv : 30;     // [31:2]
            } b;
        };

        IPDATA_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
    };

 private:
    AttributeType fifoSize_;
    AttributeType irqctrl_;
    AttributeType irqidrx_;
    AttributeType irqidtx_;
    AttributeType clock_;
    AttributeType cmdexec_;
    AttributeType listeners_;  // non-registering attribute

    ICmdExecutor *icmdexec_;
    IClock *iclk_;
    IIrqController *iirq_;

    char *rxfifo_;
    char *p_rx_wr_;
    char *p_rx_rd_;
    uint32_t rx_total_;

    static const int FIFOSZ = 15;
    char tx_fifo_[FIFOSZ];
    uint32_t tx_wcnt_;
    uint32_t tx_total_;

    mutex_def mutexListeners_;
    UartCmdType *pcmd_;

    TXDATA_TYPE txdata_;
    RXDATA_TYPE rxdata_;
    TXCTRL_TYPE txctrl_;
    RXCTRL_TYPE rxctrl_;
    IEDATA_TYPE ie_;
    IPDATA_TYPE ip_;
    SCALER_TYPE scaler_;
    FWCPUID_TYPE fwcpuid_;
    int t_cb_cnt_;
};

DECLARE_CLASS(UART)

}  // namespace debugger
