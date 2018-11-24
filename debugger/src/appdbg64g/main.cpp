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

#include "api_core.h"
#include "iservice.h"
#include "coreservices/ilink.h"
#include "coreservices/ithread.h"
#include "coreservices/icpugen.h"
#include "coreservices/icpuriscv.h"
#include "coreservices/icmdexec.h"
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
        ILink *iudp1 = static_cast<ILink *>
                (RISCV_get_service_iface("udpboard", IFACE_LINK));
        ILink *iudp2 = static_cast<ILink *>
                (RISCV_get_service_iface("udpedcl", IFACE_LINK));

        AttributeType t1;
        iudp1->getConnectionSettings(&t1);
        iudp2->setConnectionSettings(&t1);
        iudp2->getConnectionSettings(&t1);
        iudp1->setConnectionSettings(&t1);
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

#if 1 // Remove it and make "Open file.." Widget
    AttributeType res;
    char tstr[1024];
    if (Config["GlobalSettings"].has_key("LoadFile")) {
        const char *default_img =
            Config["GlobalSettings"]["LoadFile"].to_string();
        ICmdExecutor *iexec_ = static_cast<ICmdExecutor *>(
            RISCV_get_service_iface("cmdexec0", IFACE_CMD_EXECUTOR));

        if (strstr(default_img, ".elf")) {
            RISCV_sprintf(tstr, sizeof(tstr),
                        "loadelf %s nocode", default_img);
            iexec_->exec(tstr, &res, false);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "loadsrec %s.s19", default_img);
            iexec_->exec(tstr, &res, false);

            RISCV_sprintf(tstr, sizeof(tstr), "loadmap %s.map", default_img);
            iexec_->exec(tstr, &res, false);
        }
    }
#endif


    /**
     * Unreset all CPUs
     */
    /*AttributeType cpu_list;
    RISCV_get_services_with_iface(IFACE_CPU_RISCV, &cpu_list);
    for (unsigned i = 0; i < cpu_list.size(); i++) {
        IService *iserv = static_cast<IService *>(cpu_list[i].to_iface());
        ICpuGeneric *icpu = static_cast<ICpuGeneric *>(
                    iserv->getInterface(IFACE_CPU_GENERIC));
        icpu->lowerSignal(CPU_SIGNAL_RESET);  // Active HIGH. Unreset CPU model.
    }*/

    if (itst != NULL) {
        /** Get plugin specific interface. */
        ISimplePlugin * itst_access = static_cast<ISimplePlugin *>(
                                itst->getInterface(IFACE_SIMPLE_PLUGIN));
        /** Call example method */
        itst_access->exampleAction(0xcafe);
    }

    RISCV_dispatcher_start();

    //const char *t1 = RISCV_get_configuration();
    //RISCV_write_json_file(configFile.to_string(), t1);
    RISCV_cleanup();
	return 0;
}
