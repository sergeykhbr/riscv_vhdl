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

#include "api_core.h"
#include "qspi_ctrl.h"

namespace debugger {

static const uint8_t CMD0 = 0;   // () software reset
static const uint8_t CMD1 = 1;   // () Initiate initialization process
static const uint8_t ACMD41 = 41; // () For only SDC. Initiate initialization process
static const uint8_t CMD8 = 8;   // () For only SDC V2. Check voltage range
static const uint8_t CMD58 = 58;  // () Read OCR
static const uint8_t CMD12 = 12;  // () Stop to read transmission
static const uint8_t CMD16 = 16;  // (Block Length[31:0]) Change R/W Block size
static const uint8_t CMD18 = 18;  // (address[31:0]) Read multiple blocks

static const uint8_t R1_RESPONSE_NO_ERROR = 0x01;   // No errors, [0] Idle state
static const uint8_t TOKEN_CMD18 = 0xFE;    // CMD17/18/24

/* Table of CRC constants - implements x^16+x^12+x^5+1 */
static const uint16_t crc16_tab[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

uint16_t crc16_ccitt(uint16_t cksum, const unsigned char *buf, size_t len)
{
	for (size_t i = 0;  i < len;  i++)
		cksum = crc16_tab[((cksum>>8) ^ *buf++) & 0xff] ^ (cksum << 8);

	return cksum;
}


QspiController::QspiController(const char *name) : RegMemBankGeneric(name),
    sckdiv(static_cast<IService *>(this), "sckdiv", 0x00),
    sckmode(static_cast<IService *>(this), "sckmode", 0x04),
    csid(static_cast<IService *>(this), "csid", 0x10),
    csdef(static_cast<IService *>(this), "csdef", 0x14),
    csmode(static_cast<IService *>(this), "csmode", 0x18),
    delay0(static_cast<IService *>(this), "delay0", 0x28),
    delay1(static_cast<IService *>(this), "delay1", 0x2C),
    fmt(static_cast<IService *>(this), "fmt", 0x40),
    txdata(static_cast<IService *>(this), "txdata", 0x48),
    rxdata(static_cast<IService *>(this), "rxdata", 0x4C),
    txmark(static_cast<IService *>(this), "txmark", 0x50),
    rxmark(static_cast<IService *>(this), "rxmark", 0x54),
    fctrl(static_cast<IService *>(this), "fctrl", 0x60),
    ffmt(static_cast<IService *>(this), "ffmt", 0x64),
    ie(static_cast<IService *>(this), "ie", 0x70),
    ip(static_cast<IService *>(this), "ip", 0x74) {

    registerInterface(static_cast<IMasterSPI *>(this));

    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("SlaveList", &slvList_);
    registerAttribute("IrqController", &irqctrl_);
    registerAttribute("IrqId", &irqid_);

    ics_ = 0;
    state_ = Idle;
    wrbytecnt_ = 0;
    rdblocksize_ = 512;
    addr_ = 0;
    rxcnt_ = 0;
    prx_rd_ = rxbuf_;
    prx_wr_ = rxbuf_;
}

void QspiController::postinitService() {
    RegMemBankGeneric::postinitService();

    iirq_ = static_cast<IIrqController *>(
        RISCV_get_service_iface(irqctrl_.to_string(),
                                     IFACE_IRQ_CONTROLLER));
    if (!iirq_) {
        RISCV_error("Can't find IIrqController interface %s",
                    irqctrl_.to_string());
    }

    if (slvList_.size()) {
        ics_ = static_cast<ISlaveSPI *>(
            RISCV_get_service_iface(slvList_[0u].to_string(),
                                         IFACE_SPI_SLAVE));
    }
    if (!ics_) {
        RISCV_error("Can't find ISlaveSPI interface %s",
                    slvList_[0u].to_string());
    }
}

void QspiController::processCommand() {
    switch (cmd_.b.cmd) {
    case CMD0:
        RISCV_info("%s", "CMD0: software reset");
        break;
    case CMD12:
        state_ = Idle;
        rxcnt_ = 0;
        prx_rd_ = prx_wr_ = rxbuf_;
        break;
    case CMD18:
        addr_ = cmd_.b.addr[0];
        addr_ = (addr_ << 8) | cmd_.b.addr[1];
        addr_ = (addr_ << 8) | cmd_.b.addr[2];
        addr_ = (addr_ << 8) | cmd_.b.addr[3];
        state_ = BulkReading;
        break;
    default:
        RISCV_error("Unsupported command: %02x", cmd_.b.cmd);
    }

    put_rx_fifo(R1_RESPONSE_NO_ERROR);
}

void QspiController::put_rx_fifo(uint8_t byte) {
    if (rxcnt_ < sizeof(rxbuf_)) {
        *prx_wr_ = byte;
        rxcnt_++;
        if (++prx_wr_ >= &rxbuf_[FIFO_SIZE]) {
            prx_wr_ = rxbuf_;
        }
    }
}

void QspiController::write_fifo(uint8_t byte) {
    if (wrbytecnt_ == 0 && (byte & 0xC0) == 0x40) {
        cmd_.u8[wrbytecnt_++] = byte;
    } else if (wrbytecnt_) {
        cmd_.u8[wrbytecnt_++] = byte;
    }

    if (wrbytecnt_ == 6) {
        wrbytecnt_ = 0;
        RISCV_info("CMD%d  %02x %02x %02x %02x  %02x",
            cmd_.b.cmd,
            cmd_.b.addr[0],
            cmd_.b.addr[1],
            cmd_.b.addr[2],
            cmd_.b.addr[3],
            cmd_.b.crc
            );
        processCommand();
    }
}

void QspiController::load_data_block() {
    if (!ics_) {
        return;
    }

    put_rx_fifo(TOKEN_CMD18);
    uint16_t crc = 0;
    if (prx_wr_ + rdblocksize_ <= &rxbuf_[FIFO_SIZE]) {
        ics_->spiRead(addr_, prx_wr_, rdblocksize_);
        crc = crc16_ccitt(crc, prx_wr_, rdblocksize_);
        prx_wr_ += rdblocksize_;
    } else {
        size_t t = &rxbuf_[FIFO_SIZE] - prx_wr_;
        ics_->spiRead(addr_, prx_wr_, t);
        crc = crc16_ccitt(crc, prx_wr_, t);
        prx_wr_ = rxbuf_;
        ics_->spiRead(addr_ + t, prx_wr_, rdblocksize_ - t);
        crc = crc16_ccitt(crc, prx_wr_, rdblocksize_ - t);
        prx_wr_ += rdblocksize_ - t;
    }
    rxcnt_ += rdblocksize_;
    addr_ += rdblocksize_;

    put_rx_fifo(static_cast<uint8_t>(crc >> 8));
    put_rx_fifo(static_cast<uint8_t>(crc));
}

uint8_t QspiController::read_fifo() {
    uint8_t ret;

    // Load data from SD-card
    if (rxcnt_ == 0 && state_ == BulkReading) {
        load_data_block();
    }

    // Reading from FIFO
    if (rxcnt_ == 0) {
        ret = 0xff;
    } else {
        ret = *prx_rd_;
        rxcnt_--;
        if (++prx_rd_ >= &rxbuf_[FIFO_SIZE]) {
            prx_rd_ = rxbuf_;
        }
    }

    return ret;
}

uint32_t QspiController::isPendingRx() {
    if (rxcnt_ == 0 && state_ == BulkReading) {
        load_data_block();
    }
    if (rxcnt_ > (rxmark.getValue().val & 0x7)) {
        return 1ul;
    }
    return 0ul;
}

uint32_t QspiController::isRxFifoEmpty() {
    uint32_t ret = 0;
    if (rxcnt_ == 0 && state_ != BulkReading) {
        ret = 1;
    }
    return ret;
}

uint32_t QspiController::QSPI_TXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    return cur_val;
}

uint32_t QspiController::QSPI_TXDATA_TYPE::aboutToWrite(uint32_t new_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    p->write_fifo(new_val & 0xFF);
    return new_val;
}

uint32_t QspiController::QSPI_RXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    if (p->isRxFifoEmpty()) {
        // when empty is set 'data' field doesn't contain a valid frame
        cur_val = 1ul << 31;
    } else {
        cur_val = p->read_fifo();
    }
    return cur_val;
}

uint32_t QspiController::QSPI_IP_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    value_type t = {0};
    t.b.rxwm = p->isPendingRx();
    return t.v;
}

uint32_t QspiController::QSPI_FCTRL_TYPE::aboutToWrite(uint32_t new_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    return new_val;
}

uint32_t QspiController::QSPI_FFMT_TYPE::aboutToWrite(uint32_t new_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    return new_val;
}

}  // namespace debugger

