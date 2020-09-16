/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "jsoncmd.h"

namespace debugger {

JsonCommands::JsonCommands(IService *parent) : TcpCommandsGen(parent) {
}

int JsonCommands::processCommand(const char *cmdbuf, int bufsz) {
    AttributeType cmd;
    cmd.from_config(rxbuf_);
    if (!cmd.is_list() || cmd.size() < 3) {
        respcnt_ = RISCV_sprintf(respbuf_, resptotal_, "%s",
                                 "wrong request format");
        return 0;
    }

    AttributeType &requestType = cmd[1];
    AttributeType &requestAction = cmd[2];
    AttributeType resp;
    resp.make_string("OK");
    uint32_t idx = cmd[0u].to_uint32();

    if (requestType.is_equal("Configuration")) {
        resp.clone(&platformConfig_);
    } else if (requestType.is_equal("Command")) {
        /** Redirect command to console directly */
        iexec_->exec(requestAction.to_string(), &resp, false);
        if (igui_) {
            igui_->externalCommand(&requestAction);
        }
    } else if (requestType.is_equal("Breakpoint")) {
        /** Breakpoints action */
        if (requestAction[0u].is_equal("Add")) {
            br_add(requestAction[1], &resp);
        } else if (requestAction[0u].is_equal("Remove")) {
            br_rm(requestAction[1], &resp);
        } else {
            resp.make_string("Wrong breakpoint command");
        }
    } else if (requestType.is_equal("Control")) {
        /** Run Control action */
        if (requestAction[0u].is_equal("GoUntil")) {
            go_until(requestAction[1], &resp);
        } else if (requestAction[0u].is_equal("GoMsec")) {
            go_msec(requestAction[1], &resp);
        } else if (requestAction[0u].is_equal("Step")) {
            step(requestAction[1].to_int(), &resp);
        } else {
            resp.make_string("Wrong control command");
        }
    } else if (requestType.is_equal("Status")) {
        /** Pump status */
        if (requestAction.is_equal("IsON")) {
            resp.make_boolean(icpufunc_->isOn());
        } else if (requestAction.is_equal("IsHalt")) {
            resp.make_boolean(icpugen_->isHalt());
        } else if (requestAction.is_equal("Steps")) {
            resp.make_uint64(iclk_->getStepCounter());
        } else if (requestAction.is_equal("TimeSec")) {
            double t1 = iclk_->getStepCounter() / iclk_->getFreqHz();
            resp.make_floating(t1);
        } else {
            resp.make_string("Wrong status command");
        }
    } else if (requestType.is_equal("Symbol")) {
        /** Symbols table conversion */
        if (requestAction[0u].is_equal("ToAddr")) {
            symb2addr(requestAction[1].to_string(), &resp);
        } else if (requestAction[0u].is_equal("FromAddr")) {
            // todo:
        } else {
            resp.make_string("Wrong symbol command");
        }
    } else {
        resp.make_list(2);
        resp[0u].make_string("ERROR");
        resp[1].make_string("Wrong command format");
    }
    resp.to_config();

    if (static_cast<int>(resp.size()) > (resptotal_ - 64)) {
        delete [] respbuf_;
        resptotal_ = resp.size() + 64;
        respbuf_ = new char[resptotal_];
    }

    respcnt_ = RISCV_sprintf(respbuf_, resptotal_, "[%d,%s]",
                             idx, resp.to_string()) + 1;
    return rxcnt_;
}

}  // namespace debugger
