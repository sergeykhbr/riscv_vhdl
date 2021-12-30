/**
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <iclass.h>
#include <iservice.h>
#include "coreservices/iirq.h"
#include "coreservices/ispi.h"
#include "generic/mapreg.h"
#include "generic/rmembank_gen1.h"

namespace debugger {

class QspiController : public RegMemBankGeneric,
                       public IMasterSPI {
 public:
    explicit QspiController(const char *name);

    /** IService interface */
    virtual void postinitService() override;

    // Common methods
    uint32_t isPendingRx();
    uint32_t isRxFifoEmpty();
    void write_fifo(uint8_t byte);
    uint8_t read_fifo();

 private:
    void put_rx_fifo(uint8_t byte);
    void load_data_block();

 protected:
    // Chip Select Default Register
    class QSPI_CSDEF_TYPE : public MappedReg32Type {
     public:
        QSPI_CSDEF_TYPE(IService *parent, const char *name, uint64_t addr)
            : MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x1;
            value_.val = hard_reset_value_;
        }
    };

    // Frame format register
    class QSPI_FMT_TYPE : public MappedReg32Type {
     public:
        QSPI_FMT_TYPE(IService *parent, const char *name, uint64_t addr)
            : MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = (0x8 << 16);        // len number of bits per frame
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t proto : 2;     // [1:0] RW SPI protocol: 0=SINGLE; 1=DUAL; 2=QUAD
                uint32_t endian : 1;    // [2] RW SPI endianness
                uint32_t dir : 1;       // [3] RW SPI I/O direction.
                uint32_t rsrv15_4 : 12; // [15:4]
                uint32_t len : 4;       // [19:16] RW Number of bits per frame
                uint32_t rsrv31_20 : 12;// [31:20]
            } b;
        };
    };

    // Transmit Data Register
    class QSPI_TXDATA_TYPE : public MappedReg32Type {
     public:
        QSPI_TXDATA_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t data : 8;      // [7:0] RW Transmit data
                uint32_t rsrv30_8 : 23; // [30:8]
                uint32_t full : 1;      // [31] RO FIFO full flag
            } b;
        };
     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
        virtual uint32_t aboutToWrite(uint32_t new_val) override;
    };

    // Receive Data Register
    class QSPI_RXDATA_TYPE : public MappedReg32Type {
     public:
        QSPI_RXDATA_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t data : 8;      // [7:0] RO Receive data
                uint32_t rsrv30_8 : 23; // [30:8]
                uint32_t empty : 1;     // [31] RW FIFO empty flag
            } b;
        };
     protected:
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
    };

    // SPI interrupt enable
    class QSPI_IE_TYPE : public MappedReg32Type {
     public:
        QSPI_IE_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t txwm : 1;      // [0] RW Transmit watermark enable
                uint32_t rxwm : 1;      // [1] RW Receive watermark enable
                uint32_t rsrv : 30;     // [31:2]
            } b;
        };

        QSPI_IE_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
    };

    // SPI interrupt pending register
    class QSPI_IP_TYPE : public MappedReg32Type {
     public:
        QSPI_IP_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {}

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t txwm : 1;      // [0] RO Transmit watermark pending
                uint32_t rxwm : 1;      // [1] RO Receive watermark pending
                uint32_t rsrv : 30;     // [31:2]
            } b;
        };

        QSPI_IP_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
        virtual uint32_t aboutToRead(uint32_t cur_val) override;
    };

    // SPI Flash interface control register
    class QSPI_FCTRL_TYPE : public MappedReg32Type {
     public:
        QSPI_FCTRL_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = 0x1;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t en : 1;      // [0] RW SPI Flash Mode select
                uint32_t rsrv : 31;   // [31:1]
            } b;
        };

        QSPI_FCTRL_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
        virtual uint32_t aboutToWrite(uint32_t new_val) override;
    };

    // SPI Flash instruction format register
    class QSPI_FFMT_TYPE : public MappedReg32Type {
     public:
        QSPI_FFMT_TYPE(IService *parent, const char *name, uint64_t addr) :
                    MappedReg32Type(parent, name, addr) {
            hard_reset_value_ = (0x3 << 16) | (0x3 << 1) | 0x1;
            value_.val = hard_reset_value_;
        }

        union value_type {
            uint32_t v;
            struct bits_type {
                uint32_t cmd_en : 1;        // [0] RW Enable sending of command
                uint32_t addr_len : 3;      // [3:1] RW Number of address bytes (0 to 4)
                uint32_t pad_cnt : 4;       // [7:4] RW Number of dummy cycles
                uint32_t cmd_proto : 2;     // [9:8] RW Protocol for transmitting command
                uint32_t addr_proto : 2;    // [11:10] RW Protocol for transmitting address and padding
                uint32_t data_proto : 2;    // [13:12] RW Protocol for receiving data bytes
                uint32_t rsrv15_14 : 2;     // [15:14]
                uint32_t cmd_code : 8;      // [23:16] RW Value of command byte
                uint32_t pad_code : 8;      // [31:24] RW First 8 bits to transmit during dummy cycles
            } b;
        };

        QSPI_FFMT_TYPE::value_type getTyped() {
            value_type ret;
            ret.v =  value_.val;
            return ret;
        }
        virtual uint32_t aboutToWrite(uint32_t new_val) override;
    };

 private:
    void processCommand();

 private:
    AttributeType cmdexec_;
    AttributeType slvList_;
    AttributeType irqctrl_;
    AttributeType irqid_;

    IIrqController *iirq_;
    ISlaveSPI *ics_;

    union SdCommandType {
        uint8_t u8[6];
        struct bits_type {
            uint8_t cmd : 6;
            uint8_t prefix_01 : 2;
            uint8_t addr[4];
            uint8_t crc;
        } b;
    } cmd_;

    enum EWrState {
        Idle,
        BulkReading,
    } state_;

    static const int FIFO_SIZE = 4096;

    int wrbytecnt_;
    int rdblocksize_;
    uint64_t addr_;
    uint8_t rxbuf_[FIFO_SIZE];
    size_t rxcnt_;
    uint8_t* prx_wr_;
    uint8_t* prx_rd_;

    MappedReg32Type sckdiv;             // [0x00] Serial clock divisor
    MappedReg32Type sckmode;            // [0x04] Serial clock mode
    MappedReg32Type csid;               // [0x10] Chip select ID
    QSPI_CSDEF_TYPE csdef;              // [0x14] Chip select default
    MappedReg32Type csmode;             // [0x18] Chip select mode
    MappedReg32Type delay0;             // [0x28] Delay control 0
    MappedReg32Type delay1;             // [0x2C] Delay control 1
    QSPI_FMT_TYPE fmt;                  // [0x40] Frame format
    QSPI_TXDATA_TYPE txdata;            // [0x48] Tx FIFO data
    QSPI_RXDATA_TYPE rxdata;            // [0x4C] Rx FIFO data
    MappedReg32Type txmark;             // [0x50] Tx FIFO watermark
    MappedReg32Type rxmark;             // [0x54] Rx FIFO watermark
    QSPI_FCTRL_TYPE fctrl;              // [0x60] SPI flash interface control (available in direct-map only)
    QSPI_FFMT_TYPE ffmt;                // [0x64] SPI flash instruction format (available in direct-map only)
    QSPI_IE_TYPE ie;                    // [0x70] SPI interrupt enable
    QSPI_IP_TYPE ip;                    // [0x74] SPI interrupt pending
};

DECLARE_CLASS(QspiController)

}  // namespace debugger
