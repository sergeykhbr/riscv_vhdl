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
    wrstate_ = WrIdle;
    rdstate_ = RdIdle;
    wrbytecnt_ = 0;
    rdbytecnt_ = 0;
    addr_ = 0;
    txcnt_ = 0;
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
    if (cmd_.b.prefix_01 != 0x1) {
        return;
    }

    switch (cmd_.b.cmd) {
    case CMD0:
        RISCV_info("%s", "CMD0: software reset");
        break;
    case CMD12:
        wrstate_ = WrIdle;
        rdstate_ = RdCmdDiscard;
        break;
    case CMD18:
        addr_ = cmd_.b.addr[1];
        addr_ = (addr_ << 8) | cmd_.b.addr[2];
        addr_ = (addr_ << 8) | cmd_.b.addr[3];
        datablockcnt_ = -1; // open ended
        wrstate_ = WrReading;
        rdstate_ = RdCmdResponse;
        break;
    default:
        RISCV_error("Unsupported command: %02x", cmd_.b.cmd);
    }
}

void QspiController::sdWrite(uint8_t byte) {
    if (wrbytecnt_ == 0 && byte != 0xFF) {
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

uint8_t QspiController::sdRead() {
    uint8_t ret = 0;
    switch (rdstate_) {
    case RdCmdResponse:
        ret = R1_RESPONSE_NO_ERROR;
        if (datablockcnt_) {
            rdstate_ = RdDataToken;
            rdblocksize_ = 512;
            rdbytecnt_ = 0;
            if (datablockcnt_ != -1) {
                // limited number of blocks
                datablockcnt_--;
            }
        } else {
            rdstate_ = RdIdle;
        }
        break;
    case RdDataToken:
        ret =  TOKEN_CMD18;
        rdstate_ = RdDataBlock;
        if (ics_) {
            ics_->spiRead(addr_, rxbuf_, rdblocksize_);
            addr_ += rdblocksize_;
        }
        break;
    case RdDataBlock:
        ret = rxbuf_[rdbytecnt_++];
        if (rdbytecnt_ == rdblocksize_) {
            rdstate_ = RdCrc;
            rdbytecnt_ = 0;
        }
        break;
    case RdCrc:
        ret = 0xFF;
        if (datablockcnt_ == 0) {
            rdstate_ = RdIdle;
        } else {
            rdstate_ = RdDataToken;
            rdbytecnt_ = 0;
            if (datablockcnt_ != -1) {
                datablockcnt_--;
            }
        }
        break;
    case RdCmdDiscard:
        ret = R1_RESPONSE_NO_ERROR;
        datablockcnt_ = 0;
        rdbytecnt_ = 0;
        rdstate_ = RdIdle;
        break;
    default:
        ret = 0x80; // empty flag
    }
    return ret;
}

uint32_t QspiController::QSPI_TXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    return cur_val;
}

uint32_t QspiController::QSPI_TXDATA_TYPE::aboutToWrite(uint32_t new_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    p->sdWrite(new_val & 0xFF);
    return new_val;
}

uint32_t QspiController::QSPI_RXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    cur_val = p->sdRead();
    return cur_val;
}

uint32_t QspiController::QSPI_IP_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    return cur_val;
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

