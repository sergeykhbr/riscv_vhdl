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

namespace debugger {

class SimplePlugin : public IService,
                     public ISimplePlugin {
public:
    SimplePlugin(const char *name) : IService("SimplePlugin") {
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
    }
    virtual void deleteService() {}

    /** ISimplePlugin interface */
    virtual int exampleAction(int val) {
        RISCV_info("This is exampleAction(): val=%08x", val);
        return 0x555;
    }

private:
    AttributeType attr1_;
};


class SimplePluginClass : public IClass {
public:
    SimplePluginClass() : IClass("SimplePluginClass") {
        RISCV_printf(this, LOG_INFO, "Plugin class registration");
    }

    virtual IService *createService(const char *obj_name) { 
        SimplePlugin *serv = new SimplePlugin(obj_name);
        AttributeType item(static_cast<IService *>(serv));
        listInstances_.add_to_list(&item);
        return serv;
    }
};

extern "C" void plugin_init(void) {
    static SimplePluginClass local_class;
}

}  // namespace debugger
