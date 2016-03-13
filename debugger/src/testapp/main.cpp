/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Test application to verify UDP/EDCL transport library.
 */

#include "api_core.h"
#include "iservice.h"
#include "coreservices/itap.h"
#include "coreservices/iboardsim.h"
/** Plugin verification */
#include "simple_plugin/isimple_plugin.h"

using namespace debugger;

/// @todo read from file
const char *config = 
"{'Services':["
    "{'Class':'UdpServiceClass','Name':'udpboard','args':{'Timeout':400}},"
    "{'Class':'UdpServiceClass','Name':'udpedcl','args':{'Timeout':1000}},"
    "{'Class':'BoardSimClass','Name':'boardsim','args':{'Transport':'udpboard','UseHW':1,'HW_IP':'192.168.1.51'}},"
    "{'Class':'EdclServiceClass','Name':'edcltap','args':{'Transport':'udpedcl'}}],"
"}";

int main(int argc, char* argv[]) {

    IService *boardsim_service;
    IService *edcltap_service;

    RISCV_init();
    RISCV_set_configuration(config);

    boardsim_service = static_cast<IService *>(RISCV_get_service("boardsim"));
    edcltap_service = static_cast<IService *>(RISCV_get_service("edcltap"));

    IBoardSim *boardsim = static_cast<IBoardSim *>
                (boardsim_service->getInterface(IFACE_BOARDSIM));
    ITap *edcltap = static_cast<ITap *>
                (edcltap_service->getInterface(IFACE_TAP));
    
    AttributeType sim_settings = boardsim->getConnectionSettings();
    AttributeType edcl_settings = edcltap->getConnectionSettings();
    boardsim->setTargetSettings(&edcl_settings);
    edcltap->setTargetSettings(&sim_settings);

    boardsim->runSimulator();

    RISCV_sleep_ms(500);

    uint32_t data[64];
    edcltap->read(0x00000000, 8*8, reinterpret_cast<uint8_t *>(data));

    RISCV_sleep_ms(1000);
    boardsim->stopSimulator();

    /** Create Service instance of the example plugins class. */
    IService *itst = static_cast<IService *>(RISCV_create_service(
                     RISCV_get_class("SimplePluginClass"), "example0", NULL));
    /** Get plugin specific interface. */
    ISimplePlugin * itst_access = static_cast<ISimplePlugin *>(
                                itst->getInterface(IFACE_SIMPLE_PLUGIN));
    /** Call example method */
    itst_access->exampleAction(0xcafe);


    const char *t1 = RISCV_get_configuration();
    RISCV_printf(NULL, LOG_DEBUG, "Config: %s\n", t1);

    RISCV_cleanup();
	return 0;
}

