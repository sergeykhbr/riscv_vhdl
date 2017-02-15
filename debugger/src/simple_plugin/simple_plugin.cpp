/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Demo plugin for the RISC-V debugger library.
 */

#include "api_core.h"
#include "iclass.h"
#include "iservice.h"
#include "isimple_plugin.h"
#include "coreservices/icommand.h"
#include "coreservices/icmdexec.h"

namespace debugger {

class CmdDemo : public ICommand  {
public:
    explicit CmdDemo(ITap *tap, ISocInfo *info) 
        : ICommand ("democmd", tap, info) {

        briefDescr_.make_string("Example of custom command implementation");
        detailedDescr_.make_string(
            "Description:\n"
            "    Demonstration command example.\n"
            "Example:\n"
            "    democmd\n");
    }

    /** ICommand */
    virtual bool isValid(AttributeType *args) {
        if ((*args)[0u].is_equal("democmd")) {
            return CMD_VALID;
        }
        return CMD_INVALID;
    }
    virtual void exec(AttributeType *args, AttributeType *res) {
        Reg64Type t1;
        tap_->read(0xFFFFF000, 4, t1.buf);
        res->make_list(2);
        (*res)[0u].make_string("Reading 0xfffff000");
        (*res)[1].make_uint64(t1.buf32[0]);
    }

private:
};

class SimplePlugin : public IService,
                     public ISimplePlugin {
public:
    SimplePlugin(const char *name) : IService(name) {
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
        AttributeType taplist, soclist, execlist;
        RISCV_get_services_with_iface(IFACE_TAP, &taplist);
        if (taplist.size() == 0) {
            return;
        }
        RISCV_get_services_with_iface(IFACE_SOC_INFO, &soclist);
        if (soclist.size() == 0) {
            return;
        }
        RISCV_get_services_with_iface(IFACE_CMD_EXECUTOR, &execlist);
        if (execlist.size() == 0) {
            return;
        }
        IService *iserv;
        iserv = static_cast<IService *>(taplist[0u].to_iface());
        ITap * itap = static_cast<ITap *>(iserv->getInterface(IFACE_TAP));

        iserv = static_cast<IService *>(soclist[0u].to_iface());
        ISocInfo *info =
            static_cast<ISocInfo *>(iserv->getInterface(IFACE_SOC_INFO));

        iserv = static_cast<IService *>(execlist[0u].to_iface());
        exec_ = static_cast<ICmdExecutor *>(
            iserv->getInterface(IFACE_CMD_EXECUTOR));
        pcmd_ = new CmdDemo(itap, info);
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
