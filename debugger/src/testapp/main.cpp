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
/** Plugin verification */
#include "simple_plugin/isimple_plugin.h"
#include <stdio.h>
#include <string>

#define JSON_CONFIG_FILE "config.json"
using namespace debugger;

/// Use it if configuration file was not found or failed.
const char *default_config = 
"{"
  "'Services':["
    "{'Class':'BoardSimClass','Instances':["
          "{'Name':'boardsim','Attr':["
                "['LogLevel',3],"
                "['Enable',true],"
                "['Transport','udpboard']]}]},"
    "{'Class':'EdclServiceClass','Instances':["
          "{'Name':'edcltap','Attr':["
                "['LogLevel',1],"
                "['Transport','udpedcl'],"
                "['seq_cnt',0]]}]},"
    "{'Class':'UdpServiceClass','Instances':["
          "{'Name':'udpboard','Attr':["
                "['LogLevel',1],"
                "['Timeout',0x190]]},"
          "{'Name':'udpedcl','Attr':["
                "['LogLevel',1],"
                "['Timeout',0x3e8],"
                "['HostIP','192.168.0.53'],"
                "['BoardIP','192.168.0.51']]}]},"
    "{'Class':'ElfLoaderServiceClass','Instances':["
          "{'Name':'loader0','Attr':["
                "['LogLevel',4],"
                "['Tap','edcltap']]}]},"
    "{'Class':'ConsoleServiceClass','Instances':["
          "{'Name':'console0','Attr':["
                "['LogLevel',4],"
                "['Enable',true],"
                "['Consumer','cmd0']]}]},"
    "{'Class':'CmdParserServiceClass','Instances':["
          "{'Name':'cmd0','Attr':["
                "['LogLevel',4],"
                "['Console','console0'],"
                "['Tap','edcltap'],"
                "['Loader','loader0'],"
                "['History',["
                     "'csr MCPUID',"
                     "'read 0xfffff004 128'"
                     "]],"
                "['ListCSR',["
                    "['MCPUID',0xf00,'CPU description'],"
                    "['MRESET',0x782],"
                    "['MEPC',0x341,'Machine exception program counter']"
                    "]]]}]},"
    "{'Class':'SimplePluginClass','Instances':["
          "{'Name':'example0','Attr':["
                "['LogLevel',4],"
                "['attr1','This is test attr value']]}]}]"
"}";

static char cfgbuf[1<<12];

int main(int argc, char* argv[]) {
    int cfgsz = 0;
    char path[1024];
    bool loadConfig = true;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-nocfg") == 0) {
                loadConfig = false;
            }
        }
    }

    RISCV_init();
    RISCV_get_core_folder(path, sizeof(path));
    std::string cfg_filename = std::string(path) 
                    + "/" + std::string(JSON_CONFIG_FILE);

    if (loadConfig) {
        cfgsz = RISCV_read_json_file(JSON_CONFIG_FILE, cfgbuf, 
                                    static_cast<int>(sizeof(cfgbuf)));
    }

    if (cfgsz == 0) {
        RISCV_set_configuration(default_config);
    } else {
        RISCV_set_configuration(cfgbuf);
    }

   
    // Connect simulator to the EDCL debugger if enabled:
    IThread *boardsim = static_cast<IThread *>
                (RISCV_get_service_iface("boardsim", IFACE_THREAD));
    if (boardsim->isEnabled()) {

        IUdp *iudp1 = static_cast<IUdp *>
                (RISCV_get_service_iface("udpboard", IFACE_UDP));
        IUdp *iudp2 = static_cast<IUdp *>
                (RISCV_get_service_iface("udpedcl", IFACE_UDP));

        AttributeType t1 = iudp1->getConnectionSettings();
        iudp2->setTargetSettings(&t1);
        t1 = iudp2->getConnectionSettings();
        iudp1->setTargetSettings(&t1);
    }

    IService *itst = static_cast<IService *>(RISCV_get_service("example1"));
    if (itst == NULL) {
        /**
         * @brief Create instance of the example plugin class.
         */
        itst = static_cast<IService *>(RISCV_create_service(
                      RISCV_get_class("SimplePluginClass"), "example1", NULL));
    }
    /** Get plugin specific interface. */
    ISimplePlugin * itst_access = static_cast<ISimplePlugin *>(
                                itst->getInterface(IFACE_SIMPLE_PLUGIN));
    /** Call example method */
    itst_access->exampleAction(0xcafe);


    // Working cycle with console:
    IThread *in = static_cast<IThread *>(
        RISCV_get_service_iface("console0", IFACE_THREAD));
    if (in) {
        while (in->isEnabled()) {
            RISCV_sleep_ms(1000);
        }
    }

    const char *t1 = RISCV_get_configuration();
    RISCV_write_json_file(cfg_filename.c_str(), t1);

    RISCV_cleanup();
	return 0;
}
