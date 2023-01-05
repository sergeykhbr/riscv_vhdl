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
    virtual void TestReset();
    virtual uint64_t IR(uint64_t iscan, int sz);
    virtual uint64_t DR(uint64_t dscan, int sz);

 protected:
    AttributeType target_;
    IJtagTap *ibitbang_;
};

DECLARE_CLASS(JTAG)

}  // namespace debugger

