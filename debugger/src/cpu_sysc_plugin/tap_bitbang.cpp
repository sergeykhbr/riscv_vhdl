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

#include "tap_bitbang.h"

namespace debugger {

TapBitBang::TapBitBang(sc_module_name name) : sc_module(name),
    i_clk("i_clk"),
    o_trst("o_trst"),
    o_tck("o_tck"),
    o_tms("o_tms"),
    o_tdo("o_tdo"),
    i_tdi("i_tdi") {

    dtm_scaler_cnt_ = 0;
    trst_ = 0;
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s_event_dtm_ready", name);
    RISCV_event_create(&event_dtm_ready_, tstr);


    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

TapBitBang::~TapBitBang() {
    RISCV_event_close(&event_dtm_ready_);
}

void TapBitBang::resetTAP(char trst, char srst) {
    trst_ = trst;
    dtm_scaler_cnt_ = 0;
    RISCV_event_clear(&event_dtm_ready_);
    RISCV_event_wait(&event_dtm_ready_);
}

void TapBitBang::setPins(char tck, char tms, char tdi) {
    tck_ = tck;
    tms_ = tms;
    tdo_ = tdi;
    dtm_scaler_cnt_ = 0;

    RISCV_event_clear(&event_dtm_ready_);
    RISCV_event_wait(&event_dtm_ready_);
}

bool TapBitBang::getTDO() {
    return i_tdi.read();
}


void TapBitBang::registers() {
    o_trst = trst_;
    o_tck = tck_;
    o_tms = tms_;
    o_tdo = tdo_;
    if (dtm_scaler_cnt_ < 3) {
        if (++dtm_scaler_cnt_ == 3) {
            RISCV_event_set(&event_dtm_ready_);
        }
    }
}


}  // namespace debugger

