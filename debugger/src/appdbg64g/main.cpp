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
#include "coreservices/icmdexec.h"
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
    RISCV_init();
    RISCV_set_current_dir();

    uint16_t tcp_port = 0;
    AttributeType databuf;
    bool nogui = false;
    bool gui = false;

    // Parse arguments:
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-c") == 0) {
                i++;
                RISCV_read_json_file(argv[i], &databuf);
            } else if (strcmp(argv[i], "-p") == 0) {
                i++;
                tcp_port = atoi(argv[i]);
            } else if (strcmp(argv[i], "--nogui") == 0) {
                nogui = true;
            } else if (strcmp(argv[i], "--gui") == 0) {
                gui = true;
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
	
	/** Disable GUI using application arguments list */
    if (nogui) {
        Config["GlobalSettings"]["GUI"].make_boolean(false);
    } else if (gui) {
        Config["GlobalSettings"]["GUI"].make_boolean(true);
    }

    /** Redefine TCP port value using application arguments list. It is useful
	 *  in a case of several Simulator instances running at the same time and
	 *  controlled remotely from the python scripts.
	 */
    if (tcp_port) {
        AttributeType &serv = Config["Services"];
        for (unsigned i = 0; i < serv.size(); i++) {
            AttributeType &inst = serv[i]["Instances"];
            for (unsigned n = 0; n < inst.size(); n++) {
                if (strcmp(inst[n]["Name"].to_string(), "rpcserver") == 0) {
                    AttributeType &attr = inst[n]["Attr"];
                    for (unsigned i = 0; i < attr.size(); i++) {
                        AttributeType &item = attr[i];
                        if (item.size() < 2 || !item[0u].is_string()) {
                            continue;
                        }
                        if (strcmp(item[0u].to_string(), "HostPort") == 0) {
                            item[1].make_int64(tcp_port);
                        }
                    }
                }
            }
        }
    }

    if (RISCV_set_configuration(&Config)) {
        printf("Error: can't instantiate configuration\n");
        return 0;
    }

    AttributeType res;
    AttributeType &initCmds = Config["GlobalSettings"]["InitCommands"];
        ICmdExecutor *iexec_ = static_cast<ICmdExecutor *>(
            RISCV_get_service_iface("cmdexec0", IFACE_CMD_EXECUTOR));

    if (initCmds.is_list()) {
        for (unsigned int i = 0; i < initCmds.size(); i++) {
            iexec_->exec(initCmds[i].to_string(), &res, false);
        }
    }

    /** Main loop */
    RISCV_dispatcher_start();
    databuf.attr_free();

    //const char *t1 = RISCV_get_configuration();
    //RISCV_write_json_file(configFile.to_string(), t1);
    RISCV_cleanup();
    return 0;
}
