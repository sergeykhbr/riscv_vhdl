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
#include "coreservices/ijtagtap.h"

namespace debugger {

class JTAG : public IService,
             public IJtag {
 public:
    explicit JTAG(const char *name);
    virtual ~JTAG();

    /** IService interface */
    virtual void postinitService();

    /** IJtag */
    virtual void resetAsync();
    virtual void resetSync();
    virtual uint32_t scanIdCode();
    virtual uint32_t scanDtmControl();
    virtual uint64_t scanDmiBus();

 private:
    uint64_t scan(uint32_t ir, uint64_t dr, int drlen);
    void initScanSequence(uint32_t ir);
    void addToScanSequence(char tms, char tdo);
    void transmitScanSequence();

 protected:
    AttributeType target_;
    IJtagTap *ibitbang_;

    static const int SCAN_LENGTH_MAX = 4096;

    static const int IRLEN = 5;
    static const uint32_t IR_IDCODE = 0x01;
    static const uint32_t IR_DTMCONTROL = 0x10;
    static const uint32_t IR_DBUS = 0x11;
    static const uint32_t IR_BYPASS = 0x1F;

    char trst_;
    char srst_;
    struct jtag_out_type {
        char tck;
        char tms;
        char tdo;
    } out_[SCAN_LENGTH_MAX];
    char tdi_[SCAN_LENGTH_MAX];
    int scanSize_;
    ETapState etapstate_;
    uint32_t tapid_;
    uint32_t ir_;
};

DECLARE_CLASS(JTAG)

}  // namespace debugger

