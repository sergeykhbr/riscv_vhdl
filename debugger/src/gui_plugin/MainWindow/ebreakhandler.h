/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
 *
 * @brief      Breakpoint feedback controller.
 * @details    This class allows to read/write DSU control operations allowing
 *             to correctly continue execution on breakpoints.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "igui.h"

namespace debugger {

class EBreakHandler : public IGuiCmdHandler {
 public:
    EBreakHandler(IGui *gui);
    ~EBreakHandler();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

    /** Write address and instruction into fetcher to skip EBREAK once */
    void skip();

 private:
    AttributeType reqReadBr_;
    AttributeType brList_;
    AttributeType reqReadNpc_;
    AttributeType respReadNpc_;
    char reqWriteMem_[256];
    AttributeType respmemWrite_;

    IGui *igui_;
    uint64_t dsu_sw_br_;
    uint64_t dsu_hw_br_;
}; 

}  // namespace debugger
