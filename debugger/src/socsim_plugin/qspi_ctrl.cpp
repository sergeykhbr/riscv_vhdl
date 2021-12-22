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
}

void QspiController::postinitService() {
    RegMemBankGeneric::postinitService();
}

uint32_t QspiController::QSPI_TXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    return cur_val;
}

uint32_t QspiController::QSPI_TXDATA_TYPE::aboutToWrite(uint32_t new_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
    return new_val;
}

uint32_t QspiController::QSPI_RXDATA_TYPE::aboutToRead(uint32_t cur_val) {
    QspiController *p = static_cast<QspiController *>(parent_);
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

