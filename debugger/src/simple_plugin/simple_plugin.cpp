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
 * @brief      Demo plugin for the RISC-V debugger library.
 */

#include "api_core.h"
#include "iclass.h"
#include "iservice.h"
#include "isimple_plugin.h"
#include "coreservices/icommand.h"
#include "coreservices/icmdexec.h"

namespace debugger {

class CmdDemo : public ICommandRiscv {
 public:
    CmdDemo(IService *parent, IJtag *ijtag)
        : ICommandRiscv(parent, "democmd", ijtag) {

        briefDescr_.make_string("Example of custom command implementation");
        detailedDescr_.make_string(
            "Description:\n"
            "    Demonstration command example.\n"
            "Example:\n"
            "    democmd\n");
    }

    /** ICommand */
    virtual int isValid(AttributeType *args) {
        if (!(*args)[0u].is_equal("democmd")) {
            return CMD_INVALID;
        }
        return CMD_VALID;
    }
    virtual void exec(AttributeType *args, AttributeType *res) {
        Reg64Type t1;
        read_memory(0xFFFFF000, 4, t1.buf);
        res->make_list(2);
        (*res)[0u].make_string("Reading 0xfffff000");
        (*res)[1].make_uint64(t1.buf32[0]);
    }
};

class SimplePlugin : public IService,
                     public ISimplePlugin {
 public:
    explicit SimplePlugin(const char *name) : IService(name) {
        /// Interface registration
        registerInterface(static_cast<ISimplePlugin *>(this));
        /// Test attribute that will be saved/restored by core
        registerAttribute("attr1", &attr1_);
        attr1_.make_string("This is test attr value");
    }
    ~SimplePlugin() {}

    /** IService interface */
    virtual void postinitService() {
        RISCV_printf(this, LOG_INFO, "Plugin post-init example: attr1_='%s'",
                                        attr1_.to_string());
        AttributeType jtaglist, execlist;
        RISCV_get_services_with_iface(IFACE_JTAG, &jtaglist);
        if (jtaglist.size() == 0) {
            return;
        }
        RISCV_get_services_with_iface(IFACE_CMD_EXECUTOR, &execlist);
        if (execlist.size() == 0) {
            return;
        }
        IService *iserv;
        iserv = static_cast<IService *>(jtaglist[0u].to_iface());
        IJtag * ijtag = static_cast<IJtag *>(iserv->getInterface(IFACE_JTAG));

        iserv = static_cast<IService *>(execlist[0u].to_iface());
        exec_ = static_cast<ICmdExecutor *>(
            iserv->getInterface(IFACE_CMD_EXECUTOR));
        pcmd_ = new CmdDemo(this, ijtag);
        exec_->registerCommand(pcmd_);
    }
    virtual void predeleteService() {
        exec_->unregisterCommand(pcmd_);
        delete pcmd_;
    }

    /** ISimplePlugin interface */
    virtual int exampleAction(int val) {
        RISCV_info("This is exampleAction(): val=%08x", val);
        return 0x555;
    }

 private:
    AttributeType attr1_;
    ICmdExecutor *exec_;
    CmdDemo *pcmd_;
};

DECLARE_CLASS(SimplePlugin)

extern "C" void plugin_init(void) {
    REGISTER_CLASS(SimplePlugin);
}

}  // namespace debugger
