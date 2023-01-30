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

#include <iface.h>
#include <inttypes.h>
#include <iservice.h>
#include "icommand.h"

namespace debugger {

class GenericDisplayCmdType : public ICommand {
 public:
    GenericDisplayCmdType(IService *parent, const char *name)
        : ICommand(parent, name) {
        briefDescr_.make_string("Display controller management command.");
        detailedDescr_.make_string(
            "Description:\n"
            "    Read display resolution using config command\n"
            "    or frame using 'frame' subcommand.\n"
            "    Additional option 'encoded' allows reduce frame buffer\n"
            "    size.\n"
            "Response config:\n"
            "    List {Width:w,Height:h,BkgColor:0x00ff00}\n"
            "Response 'frame':\n"
            "    List [b0,b1,b1,...],\n"
            "          Bytes of column0,column1,etcn"
            "Usage:\n"
            "    display0 config\n"
            "    display0 frame\n"
            "    display0 frame encoded");
    }

    /** ICommand */
    virtual int isValid(AttributeType *args) {
        if (!cmdName_.is_equal((*args)[0u].to_string())) {
            return CMD_INVALID;
        }
        if (args->size() < 2) {
            return CMD_WRONG_ARGS;
        }
        return CMD_VALID;
    }
    virtual void exec(AttributeType *args, AttributeType *res){
        AttributeType &type = (*args)[1];
        if (type.is_equal("config")) {
            res->make_dict();
            (*res)["Width"].make_int64(getWidth());
            (*res)["Height"].make_int64(getHeight());
            (*res)["BkgColor"].make_uint64(getBkgColor());
        } else if (type.is_equal("frame")) {
            bool diff = false;
            if (args->size() > 2 && (*args)[2].is_equal("diff")) {
                diff = true;
            }
            getFrame(res, diff);
            if (args->size() > 2 && (*args)[2].is_equal("encoded")) {
                encode(res);
            }
        }
    }

 protected:
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual uint32_t getBkgColor() = 0;     // distance between pixels
    virtual void getFrame(AttributeType *res, bool diff) = 0;
    virtual void encode(AttributeType *frame) {
        if (!frame->is_data()) {
            return;
        }
        unsigned sz = frame->size();
        uint32_t *frameTempProxy_ = new uint32_t[(sz + 3)/sizeof(uint32_t)];
        memcpy(frameTempProxy_, frame->data(), sz);
        sz /= sizeof(uint32_t);
        frame->make_list(sz);
        for (unsigned i = 0; i < sz; i++) {
            (*frame)[i].make_uint64(frameTempProxy_[i]);
        }
        delete [] frameTempProxy_;
    }
};

}  // namespace debugger
