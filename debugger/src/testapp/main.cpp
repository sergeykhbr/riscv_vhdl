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
#include "coreservices/ielfloader.h"
/** Plugin verification */
#include "simple_plugin/isimple_plugin.h"
#include "socsim_plugin/iboardsim.h"
#include <stdio.h>

#define JSON_CONFIG_FILE "config.json"
using namespace debugger;

/// Use it if configuration file was not found or failed.
const char *default_config = 
"{"
  "'Services':["
    "{'Class':'BoardSimClass','Instances':["
          "{'Name':'boardsim','Attr':["
                "['LogLevel',4],"
                "['Disable',true],"
                "['Transport','udpboard']]}]},"
    "{'Class':'EdclServiceClass','Instances':["
          "{'Name':'edcltap','Attr':["
                "['LogLevel',4],"
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
    "{'Class':'SimplePluginClass','Instances':["
          "{'Name':'example0','Attr':["
                "['LogLevel',4],"
                "['attr1','This is test attr value']]}]}]"
"}";

static char cfgbuf[1<<12];

int main(int argc, char* argv[]) {
#if 0
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

    IBoardSim *boardsim = static_cast<IBoardSim *>
                (RISCV_get_service_iface("boardsim", IFACE_BOARDSIM));
    ITap *edcltap = static_cast<ITap *>
                (RISCV_get_service_iface("edcltap", IFACE_TAP));
    
    // Connect simulator to the EDCL debugger:
    if (!boardsim->isDisabled()) {
        IUdp *iudp1 = static_cast<IUdp *>
                (RISCV_get_service_iface("udpboard", IFACE_UDP));
        IUdp *iudp2 = static_cast<IUdp *>
                (RISCV_get_service_iface("udpedcl", IFACE_UDP));

        AttributeType t1 = iudp1->getConnectionSettings();
        iudp2->setTargetSettings(&t1);
        t1 = iudp2->getConnectionSettings();
        iudp1->setTargetSettings(&t1);
    }

    IElfLoader *iloader = static_cast<IElfLoader *>
        (RISCV_get_service_iface("loader0", IFACE_ELFLOADER));
    iloader->loadFile("bootimage");

    // 1. Dump boot ROM image.
    //      Base Address = 0x00000000
    //      Memory size  = 8 KB
    uint32_t boot[2048];
    FILE *fbootimage = fopen("bootimage.hex", "w");
    edcltap->read(0x00000000, 8*1024, reinterpret_cast<uint8_t *>(boot));
    char boot_str[256];
    int len;
    for (int i = 0; i < 512; i++) {
        len = sprintf(boot_str, "%08x%08x%08x%08x\n",
                      boot[4*i + 3], boot[4*i + 2], boot[4*i + 1], boot[4*i]);
        fwrite(boot_str, len, 1, fbootimage);
    }
    fclose(fbootimage);


#if 0
    for (int i = 0; i < 1024; i++) {
        boot[i] = 0xfeed0000 + i;
    }
    edcltap->write(0x10040000, 4096, reinterpret_cast<uint8_t *>(boot));
    edcltap->read(0x10040000, 4096, reinterpret_cast<uint8_t *>(&boot[1024]));
    for (int i = 0; i < 1024; i++) {
        if (boot[i] != boot[i + 1024]) {
            printf("Write error: %08x != %08x\n", boot[i], boot[i + 1024]);
        }
    }
#endif

    // 2. Write/read Firmware identificator
    uint32_t fwid = 0x20160322;
    edcltap->write(0xfffff004, 4, reinterpret_cast<uint8_t *>(&fwid));
    uint32_t fwid_chk;
    edcltap->read(0xfffff004, 4, reinterpret_cast<uint8_t *>(&fwid_chk));
    printf("FWID <= %08x; chk = %08x\n", fwid, fwid_chk);


    // 3. Read CSR MCPUID value
    //      DSU Base address 0x80080000
    //      Each CSR register value has 128-bits alignment.
    #define CSR_MCPUID 0xf00
    uint64_t mcpuid;
    edcltap->read(0x80080000 + (CSR_MCPUID << 4), 8, 
                        reinterpret_cast<uint8_t *>(&mcpuid));
    static const char *CPU_BASE[4] = {"RV32I", "RV32E", "RV64I", "RV128I"};
    printf("CPUID = %08x%08x\n", static_cast<uint32_t>(mcpuid >> 32),
                                 static_cast<uint32_t>(mcpuid));
    printf("    Base: %s", CPU_BASE[(mcpuid >> 62) & 0x3]);
    // total 26 extensions
    char extenstion[2] = {0};
    for (int i = 0; i < 26; i++) {
        if (mcpuid & (1 << i)) {
            extenstion[0] = 'A' + i;
            if (extenstion[0] == 'I') {
                continue;
            }
            printf("%s", extenstion);
        }
    }
    printf("\n");


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


    const char *t1 = RISCV_get_configuration();
    RISCV_write_json_file(JSON_CONFIG_FILE, t1);

    RISCV_cleanup();
	return 0;
}
