/*
 *  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "api_core.h"
#include "coreservices/ijtagbitbang.h"
#include <systemc.h>

namespace debugger {

class TapBitBang : public sc_module,
                   public IJtagBitBang {
 public:
    sc_in<bool> i_clk;   // system clock
    sc_out<bool> o_trst; // Must be open-train, pullup
    sc_out<bool> o_tck;
    sc_out<bool> o_tms;
    sc_out<bool> o_tdo;
    sc_in<bool> i_tdi;

    void registers();

    SC_HAS_PROCESS(TapBitBang);

    TapBitBang(sc_module_name name);
    virtual ~TapBitBang();

    /** IJtagTap */
    virtual void resetTAP(char trst, char srst);
    virtual void setPins(char tck, char tms, char tdi);
    virtual bool getTDO();

 private:

    event_def event_dtm_ready_;

    char trst_;
    char tck_;
    char tms_;
    char tdo_;
    int dtm_scaler_cnt_;
};

}  // namespace debugger

