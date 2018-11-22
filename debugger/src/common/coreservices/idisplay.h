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
 */

#ifndef __DEBUGGER_PLUGIN_IDISPLAY_H__
#define __DEBUGGER_PLUGIN_IDISPLAY_H__

#include <iface.h>
#include <inttypes.h>
#include <iservice.h>
#include "icommand.h"

namespace debugger {

class GenericDisplayCmdType : public ICommand {
 public:
    GenericDisplayCmdType(IService *parent, const char *name)
        : ICommand(name, 0) {
        parent_ = parent;
        briefDescr_.make_string("Display controller management command.");
        detailedDescr_.make_string(
            "Description:\n"
            "    Read display resolution using config command\n"
            "    or frame using 'frame' sucommand.\n"
            "Response config:\n"
            "    List {Width:w,Height:h,BkgColor:0x00ff00}\n"
            "Response 'frame':\n"
            "    List [b0,b1,b1,...],\n"
            "          Bytes of column0,column1,etcn"
            "Usage:\n"
            "    display0 config\n"
            "    display0 frame");
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
        }
    }

 protected:
    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
    virtual uint32_t getBkgColor() = 0;     // distance between pixels
    virtual void getFrame(AttributeType *res, bool diff) = 0;
 protected:
    IService *parent_;
};


}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IDISPLAY_H__
