/*
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

#include "boardsim.h"
#include "gpio.h"
#include "uart.h"
#include "pnp.h"
#include "gnss_stub.h"
#include "gptimers.h"
#include "rfctrl.h"
#include "fsev2.h"
#include "uartmst.h"
#include "hardreset.h"
#include "fpu_func.h"
#include "plic.h"
#include "clint.h"
#include "ddr_phys_flt.h"
#include "ddr_ctrl.h"
#include "prci.h"
#include "qspi_ctrl.h"
#include "otp.h"
#include "sdcard.h"
#include "ddr.h"

namespace debugger {

extern "C" void plugin_init(void) {
    REGISTER_CLASS_IDX(BoardSim, 1);
    REGISTER_CLASS_IDX(PLIC, 2);
    REGISTER_CLASS_IDX(GPIO, 3);
    REGISTER_CLASS_IDX(UART, 4);
    REGISTER_CLASS_IDX(PNP, 5);
    REGISTER_CLASS_IDX(CLINT, 6);
    REGISTER_CLASS_IDX(GNSSStub, 7);
    REGISTER_CLASS_IDX(GPTimers, 9);
    REGISTER_CLASS_IDX(RfController, 10);
    REGISTER_CLASS_IDX(FseV2, 11);
    REGISTER_CLASS_IDX(UartMst, 12);
    REGISTER_CLASS_IDX(HardReset, 13);
    REGISTER_CLASS_IDX(FpuFunctional, 14);
    REGISTER_CLASS_IDX(DdrPhysFilter, 15);
    REGISTER_CLASS_IDX(DdrController, 16);
    REGISTER_CLASS_IDX(PRCI, 17);
    REGISTER_CLASS_IDX(QspiController, 18);
    REGISTER_CLASS_IDX(OTP, 19);
    REGISTER_CLASS_IDX(SdCard, 20);
    REGISTER_CLASS_IDX(DDR, 21);
}

}  // namespace debugger
