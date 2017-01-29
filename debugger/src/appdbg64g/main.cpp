/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Test application to verify UDP/EDCL transport library.
 */

#include "api_core.h"
#include "iservice.h"
#include "coreservices/iudp.h"
#include "coreservices/ithread.h"
#include "coreservices/icpuriscv.h"
/** Plugin verification */
#include "simple_plugin/isimple_plugin.h"
#include <stdio.h>
#include <string>

using namespace debugger;

static AttributeType Config;

const AttributeType *getConfigOfService(const AttributeType &cfg, 
                                        const char *name) {
    const AttributeType &serv = cfg["Services"];
    for (unsigned i = 0; i < serv.size(); i++) {
        const AttributeType &inst = serv[i]["Instances"];
        for (unsigned n = 0; n < inst.size(); n++) {
            if (strcmp(inst[n]["Name"].to_string(), name) == 0) {
                return &inst[n];
            }
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    AttributeType scriptFile("");
    AttributeType databuf;

    RISCV_init();
    RISCV_set_current_dir();

    // Parse arguments:
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-c") == 0) {
                i++;
                RISCV_read_json_file(argv[i], &databuf);
            } else if (strcmp(argv[i], "-script") == 0) {
                i++;
                scriptFile.make_string(argv[i]);
            }
        }
    }

    if (databuf.size() == 0) {
        printf("Error: Platform script file not defined\n");
        printf("       Use -c key to specify configuration file location:\n");
        printf("Example: appdbg64.exe -c ../../targets/default.json\n");
        return 0;
    }

    Config.from_config(databuf.to_string());
    Config["GlobalSettings"]["ScriptFile"] = scriptFile;

    if (RISCV_set_configuration(&Config)) {
        printf("Error: can't instantiate configuration\n");
        return 0;
    }
   
    // Connect simulator to the EDCL debugger if enabled:
    if (Config["GlobalSettings"]["SimEnable"].to_bool()) {
        IUdp *iudp1 = static_cast<IUdp *>
                (RISCV_get_service_iface("udpboard", IFACE_UDP));
        IUdp *iudp2 = static_cast<IUdp *>
                (RISCV_get_service_iface("udpedcl", IFACE_UDP));

        AttributeType t1 = iudp1->getConnectionSettings();
        iudp2->setTargetSettings(&t1);
        t1 = iudp2->getConnectionSettings();
        iudp1->setTargetSettings(&t1);
    }

    IService *itst = static_cast<IService *>(RISCV_get_service("example0"));
    if (itst == NULL) {
        /**
         * @brief Create instance of the example plugin class.
         */
        IFace *simple = RISCV_get_class("SimplePluginClass");
        if (simple) {
	        itst = static_cast<IService *>(RISCV_create_service(
                          simple, "example0", NULL));
        }
    }

    /**
     * Unreset all CPUs
     */
    AttributeType cpu_list;
    RISCV_get_services_with_iface(IFACE_CPU_RISCV, &cpu_list);
    for (unsigned i = 0; i < cpu_list.size(); i++) {
        IService *iserv = static_cast<IService *>(cpu_list[i].to_iface());
        ICpuRiscV *icpu = static_cast<ICpuRiscV *>(
                    iserv->getInterface(IFACE_CPU_RISCV));
        icpu->lowerSignal(CPU_SIGNAL_RESET);  // Active HIGH. Unreset CPU model.
    }

    if (itst != NULL) {
        /** Get plugin specific interface. */
        ISimplePlugin * itst_access = static_cast<ISimplePlugin *>(
                                itst->getInterface(IFACE_SIMPLE_PLUGIN));
        /** Call example method */
        itst_access->exampleAction(0xcafe);
    }

    // Working cycle with console:
    while (RISCV_is_active()) {
        RISCV_sleep_ms(100);
    }

    //const char *t1 = RISCV_get_configuration();
    //RISCV_write_json_file(configFile.to_string(), t1);

    RISCV_cleanup();
	return 0;
}
