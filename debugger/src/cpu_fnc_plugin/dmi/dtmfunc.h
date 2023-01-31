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

#pragma once

#include "api_core.h"
#include "iclass.h"
#include "iservice.h"
#include "coreservices/ireset.h"
#include "coreservices/ijtagbitbang.h"
#include "coreservices/idmi.h"

namespace debugger {

class DtmFunctional : public IService,
                      public IJtagBitBang {

 public:
    explicit DtmFunctional(const char* name);
    virtual ~DtmFunctional();

    /** IService interface */
    void postinitService() override;

    /** IJtagTap interface */
    virtual void resetTAP(char trst, char srst);
    virtual void setPins(char tck, char tms, char tdi);
    virtual bool getTDO();

 private:
    AttributeType version_;
    AttributeType idcode_;
    AttributeType irlen_;
    AttributeType abits_;
    AttributeType dmi_;

    IDmi *idmi_;

    char tck_;
    char tms_;
    char tdi_;

    typedef enum {
        RESET_TAP,
        IDLE,
        SELECT_DR_SCAN,
        CAPTURE_DR,
        SHIFT_DR,
        EXIT1_DR,
        PAUSE_DR,
        EXIT2_DR,
        UPDATE_DR,
        SELECT_IR_SCAN,
        CAPTURE_IR,
        SHIFT_IR,
        EXIT1_IR,
        PAUSE_IR,
        EXIT2_IR,
        UPDATE_IR
    } jtag_state_t;

    const jtag_state_t next[16][2] = {
        /* TEST_LOGIC_RESET */    { IDLE, RESET_TAP },
        /* RUN_TEST_IDLE */       { IDLE, SELECT_DR_SCAN },
        /* SELECT_DR_SCAN */      { CAPTURE_DR, SELECT_IR_SCAN },
        /* CAPTURE_DR */          { SHIFT_DR, EXIT1_DR },
        /* SHIFT_DR */            { SHIFT_DR, EXIT1_DR },
        /* EXIT1_DR */            { PAUSE_DR, UPDATE_DR },
        /* PAUSE_DR */            { PAUSE_DR, EXIT2_DR },
        /* EXIT2_DR */            { SHIFT_DR, UPDATE_DR },
        /* UPDATE_DR */           { IDLE, SELECT_DR_SCAN },
        /* SELECT_IR_SCAN */      { CAPTURE_IR, RESET_TAP },
        /* CAPTURE_IR */          { SHIFT_IR, EXIT1_IR },
        /* SHIFT_IR */            { SHIFT_IR, EXIT1_IR },
        /* EXIT1_IR */            { PAUSE_IR, UPDATE_IR },
        /* PAUSE_IR */            { PAUSE_IR, EXIT2_IR },
        /* EXIT2_IR */            { SHIFT_IR, UPDATE_IR },
        /* UPDATE_IR */           { IDLE, SELECT_DR_SCAN }
    };

    enum {
      IR_IDCODE=1,
      IR_DTMCONTROL=0x10,
      IR_DBUS=0x11,
      IR_BYPASS=0x1f
    };


    jtag_state_t estate_;
    uint64_t dr_;
    uint64_t ir_;
    uint64_t bypass_;
    int dr_length_;

    uint32_t dmi_addr_;
    uint32_t dmi_data_;
    uint32_t dmi_status_;
};

DECLARE_CLASS(DtmFunctional)

}  // namespace debugger

