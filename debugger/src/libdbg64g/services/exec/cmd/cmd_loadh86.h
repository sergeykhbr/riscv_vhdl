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

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdLoadH86 : public ICommandRiscv  {
 public:
    explicit CmdLoadH86(IService *parent, IJtag *ijtag);

    /** ICommand interface */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

 private:
    uint8_t str2byte(uint8_t *pair);
    bool check_crc(uint8_t *str, int sz);
    int readline(uint8_t *img, int &off,
                 uint64_t &addr, int &sz, uint8_t *out);

 private:
    char header_data_[1024];
    int addr_msb_;
};

}  // namespace debugger
