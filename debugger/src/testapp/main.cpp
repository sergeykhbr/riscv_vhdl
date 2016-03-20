/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Test application to verify UDP/EDCL transport library.
 */

#include "api_core.h"
#include "iservice.h"
#include "coreservices/iudp.h"
#include "coreservices/itap.h"
#include "coreservices/iboardsim.h"
/** Plugin verification */
#include "simple_plugin/isimple_plugin.h"
#include <stdio.h>

#define JSON_CONFIG_FILE "config.json"
using namespace debugger;

/// Use it if configuration file was not found or failed.
const char *default_config = 
"{"
  "'Services':["
    "{'Class':'BoardSimClass','Instances':["
          "{'Name':'boardsim','Attr':["
                "['Disable',false],"
                "['Transport','udpboard']]}]},"
    "{'Class':'EdclServiceClass','Instances':["
          "{'Name':'edcltap','Attr':["
                "['Transport','udpedcl'],"
                "['seq_cnt',0]]}]},"
    "{'Class':'UdpServiceClass','Instances':["
          "{'Name':'udpboard','Attr':["
                "['Timeout',0x190]]},"
          "{'Name':'udpedcl','Attr':["
                "['Timeout',0x3e8],"
                "['HostIP','192.168.0.53'],"
                "['BoardIP','192.168.0.51']]}]},"
    "{'Class':'SimplePluginClass','Instances':[{'Name':'example0',"
        "'Attr':[['attr1','This is test attr value']]}]}]"
"}";

static char cfgbuf[1<<12];

int main(int argc, char* argv[]) {

    IService *boardsim_service;
    IService *edcltap_service;

#if 1
    int cfgsz = 0;
#else
    int cfgsz = RISCV_read_json_file(JSON_CONFIG_FILE, cfgbuf, 
                                    static_cast<int>(sizeof(cfgbuf)));
#endif

    RISCV_init();
    if (cfgsz == 0) {
        RISCV_set_configuration(default_config);
    } else {
        RISCV_set_configuration(cfgbuf);
    }

    boardsim_service = static_cast<IService *>(RISCV_get_service("boardsim"));
    edcltap_service = static_cast<IService *>(RISCV_get_service("edcltap"));

    IBoardSim *boardsim = static_cast<IBoardSim *>
                (boardsim_service->getInterface(IFACE_BOARDSIM));
    ITap *edcltap = static_cast<ITap *>
                (edcltap_service->getInterface(IFACE_TAP));
    
    if (!boardsim->isDisabled()) {
        IUdp *iudp1 = static_cast<IUdp *>(static_cast<IService *>(
                RISCV_get_service("udpboard"))->getInterface(IFACE_UDP));
        IUdp *iudp2 = static_cast<IUdp *>(static_cast<IService *>(
                RISCV_get_service("udpedcl"))->getInterface(IFACE_UDP));

        iudp2->setTargetSettings(&iudp1->getConnectionSettings());
        iudp1->setTargetSettings(&iudp2->getConnectionSettings());

        boardsim->runSimulator();
    }


    uint32_t data[64];
    while (1) {
        edcltap->read(0x00000000, 8*8, reinterpret_cast<uint8_t *>(data));
        RISCV_sleep_ms(1000);
    }

    if (!boardsim->isDisabled()) {
        boardsim->stopSimulator();
    }

    IService *itst = static_cast<IService *>(RISCV_get_service("example1"));
    if (itst == NULL) {
        /**
         * @brief Create instance of the example plugins class.
         */
        itst = static_cast<IService *>(RISCV_create_service(
                      RISCV_get_class("SimplePluginClass"), "example1", NULL));
    }
    /** Get plugin specific interface. */
    ISimplePlugin * itst_access = static_cast<ISimplePlugin *>(
                                itst->getInterface(IFACE_SIMPLE_PLUGIN));
    /** Call example method */
    itst_access->exampleAction(0xcafe);


    const char *t1 = RISCV_get_configuration();
    RISCV_write_json_file(JSON_CONFIG_FILE, t1);

    RISCV_cleanup();
	return 0;
}
