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

#include "ebreakhandler.h"
#include "coreservices/isrccode.h"
#include "debug/dsumap.h"

namespace debugger {

EBreakHandler::EBreakHandler(IGui *gui) {
    igui_ = gui;
    reqDCSR_.make_string("csr 0x7b0");
    reqReadBr_.make_string("br");
    reqReadNpc_.make_string("reg npc");
    estate_ = State_Idle;

    dsu_sw_br_ = DSUREGBASE(udbg.v.br_address_fetch);
    dsu_hw_br_ = DSUREGBASE(udbg.v.remove_breakpoint);
}

EBreakHandler::~EBreakHandler() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void EBreakHandler::skip() {
    estate_ = State_Cause;
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            reqDCSR_.to_string(), &respDCSR_, true);
}

void EBreakHandler::handleResponse(const char *cmd) {
    uint64_t br_addr;
    uint32_t br_instr;
    uint64_t br_flags;
    uint64_t cause;

    switch (estate_) {
    case State_Cause:
        estate_ = State_Idle;
        if (reqDCSR_.is_equal(cmd) ) {
            cause = (respDCSR_.to_uint64() >> 6) & 0x7;     // dcsr[8:6] cause field
            if (cause == 1 || cause == 2) {                 // 1=ebreak; 2=hw trigger
                estate_ = State_npc;
                igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                                        reqReadNpc_.to_string(), &respReadNpc_, true);
            }
        }
        break;
    case State_npc:
        if (reqReadNpc_.is_equal(cmd) ) {
            estate_ = State_CheckBreakpoint;
            igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                                    reqReadBr_.to_string(), &brList_, true);
        } else {
            estate_ = State_Idle;
        }
        break;
    case State_CheckBreakpoint:
        estate_ = State_Idle;
        br_addr = respReadNpc_.to_uint64();
        br_instr = 0;
        br_flags;
        for (unsigned i = 0; i < brList_.size(); i++) {
            const AttributeType &br = brList_[i];
            if (br_addr == br[BrkList_address].to_uint64()) {
                br_instr = br[BrkList_instr].to_int();
                br_flags = br[BrkList_flags].to_uint64();
                break;
            }
        }
        if (br_instr) {
            if (br_flags & BreakFlag_HW) {
                RISCV_sprintf(reqWriteMem_, sizeof(reqWriteMem_),
                        "write 0x%08" RV_PRI64 "x 8 0x%" RV_PRI64 "x",
                        dsu_hw_br_, br_addr);
            } else {
                RISCV_sprintf(reqWriteMem_, sizeof(reqWriteMem_),
                        "write 0x%08" RV_PRI64 "x 16 [0x%" RV_PRI64 "x,0x%x]",
                        dsu_sw_br_, br_addr, br_instr);
            }
            igui_->registerCommand(NULL, reqWriteMem_, &respmemWrite_, true);
        }
        break;
    default:;
    }
}

}  // namespace debugger
