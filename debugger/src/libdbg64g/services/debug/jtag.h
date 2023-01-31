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

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ijtag.h"
#include "coreservices/ijtagbitbang.h"

namespace debugger {

class JTAG : public IService,
             public IJtag {
 public:
    explicit JTAG(const char *name);
    virtual ~JTAG();

    /** IService interface */
    virtual void postinitService() override;

    /** IJtag */
    virtual void resetAsync();
    virtual void resetSync();
    virtual uint32_t scanIdCode();
    virtual IJtag::DtmcsType scanDtmcs();
    virtual uint32_t scanDmi(uint32_t addr, uint32_t data, IJtag::EDmiOperation op);

 private:
    uint64_t scan(uint32_t ir, uint64_t dr, int drlen);
    void initScanSequence(uint32_t ir);
    void addToScanSequence(char tms, char tdo, ETapState nextstate);
    void endScanSequenceToIdle();

    void transmitScanSequence();
    uint64_t getRxData();

 protected:
    AttributeType targetBitBang_;
    IJtagBitBang *ibb_;

    static const int SCAN_LENGTH_MAX = 4096;

    static const int IRLEN = 5;
    static const int ABITS = 7;     // should be checked in dtmconctrol register

    char trst_;
    char srst_;
    struct jtag_out_type {
        char tck;
        char tms;
        char tdo;
        ETapState state;
    } out_[SCAN_LENGTH_MAX];
    char tdi_[SCAN_LENGTH_MAX];
    int scanSize_;
    ETapState etapstate_;
    uint32_t tapid_;
    uint32_t ir_;
    uint64_t drshift_;
};

DECLARE_CLASS(JTAG)

}  // namespace debugger

