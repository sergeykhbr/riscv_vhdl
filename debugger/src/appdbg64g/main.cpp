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
  "'GlobalSettings':{"
    "'SimEnable':true,"
    "'GUI':'false',"
    "'ScriptFile':''"
  "},"
  "'Services':["
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
                "['Tap','edcltap'],"
                "['VerifyEna',true]]}]},"
    "{'Class':'ConsoleServiceClass','Instances':["
          "{'Name':'console0','Attr':["
                "['LogLevel',4],"
                "['Enable',true],"
                "['LogFile','test.log'],"
                "['Serial','uart0'],"
                "['Consumer','cmd0']]}]},"
    "{'Class':'CmdParserServiceClass','Instances':["
          "{'Name':'cmd0','Attr':["
                "['LogLevel',4],"
                "['Console','console0'],"
                "['Tap','edcltap'],"
                "['Loader','loader0'],"
                "['HistorySize',64],"
                "['History',["
                     "'csr MCPUID',"
                     "'csr MTIME',"
                     "'read 0xfffff004 128',"
                     "'loadelf helloworld'"
                     "]],"
                "['RegNames',[['zero',0],['ra',1],['sp',2],['gp',3],"
                            "['tp',4],['t0',5],['t1',6],['t2',7],"
                            "['s0',8],['s1',9],['a0',10],['a1',11],"
                            "['a2',12],['a3',13],['a4',14],['a5',15],"
                            "['a6',16],['a7',17],['s2',18],['s3',19],"
                            "['s4',20],['s5',21],['s6',22],['s7',23],"
                            "['s8',24],['s9',25],['s10',26],['s11',27],"
                            "['t3',28],['t4',29],['t5',30],['t6',31],"
                            "['pc',32],['npc',33]]],"
                "['ListCSR',["
                    "['MCPUID',0xf00,'CPU description'],"
                    "['MRESET',0x782],"
                    "['MTIME',0x701,'Machine wall-clock time.'],"
                    "['MEPC',0x341,'Machine exception program counter']"
                    "]]]}]},"
    "{'Class':'SimplePluginClass','Instances':["
          "{'Name':'example0','Attr':["
                "['LogLevel',4],"
                "['attr1','This is test attr value']]}]},"
    "{'Class':'CpuRiscV_FunctionalClass','Instances':["
          "{'Name':'core0','Attr':["
                "['LogLevel',4],"
                "['Bus','axi0'],"
                "['ListExtISA',['I','M','A']],"
                "['FreqHz',60000000]"
                "]}]},"
    "{'Class':'MemorySimClass','Instances':["
          "{'Name':'bootrom0','Attr':["
                "['LogLevel',1],"
                "['InitFile','../../../rocket_soc/fw_images/bootimage.hex'],"
                "['ReadOnly',true],"
                "['BaseAddress',0x0],"
                "['Length',8192]"
                "]}]},"
    "{'Class':'MemorySimClass','Instances':["
          "{'Name':'fwimage0','Attr':["
                "['LogLevel',1],"
                "['InitFile','../../../rocket_soc/fw_images/fwimage.hex'],"
                "['ReadOnly',true],"
                "['BaseAddress',0x00100000],"
                "['Length',0x40000]"
                "]}]},"
    "{'Class':'MemorySimClass','Instances':["
          "{'Name':'sram0','Attr':["
                "['LogLevel',1],"
                "['InitFile','../../../rocket_soc/fw_images/fwimage.hex'],"
                "['ReadOnly',false],"
                "['BaseAddress',0x10000000],"
                "['Length',0x80000]"
                "]}]},"
    "{'Class':'GPIOClass','Instances':["
          "{'Name':'gpio0','Attr':["
                "['LogLevel',3],"
                "['BaseAddress',0x80000000],"
                "['Length',4096],"
                "['DIP',0x1]"
                "]}]},"
    "{'Class':'UARTClass','Instances':["
          "{'Name':'uart0','Attr':["
                "['LogLevel',1],"
                "['BaseAddress',0x80001000],"
                "['Length',4096],"
                "['IrqLine',1],"
                "['IrqControl','irqctrl0']"
                "]}]},"
    "{'Class':'IrqControllerClass','Instances':["
          "{'Name':'irqctrl0','Attr':["
                "['LogLevel',1],"
                "['BaseAddress',0x80002000],"
                "['Length',4096],"
                "['HostIO','core0'],"
                "['CSR_MIPI',0x783]"
                "]}]},"
    "{'Class':'DSUClass','Instances':["
          "{'Name':'dsu0','Attr':["
                "['LogLevel',1],"
                "['BaseAddress',0x80080000],"
                "['Length',0x20000],"
                "['HostIO','core0']"
                "]}]},"
    "{'Class':'GNSSStubClass','Instances':["
          "{'Name':'gnss0','Attr':["
                "['LogLevel',1],"
                "['BaseAddress',0x80003000],"
                "['Length',4096],"
                "['IrqLine',0],"
                "['IrqControl','irqctrl0'],"
                "['ClkSource','core0']"
                "]}]},"
    "{'Class':'GPTimersClass','Instances':["
          "{'Name':'gptmr0','Attr':["
                "['LogLevel',1],"
                "['BaseAddress',0x80005000],"
                "['Length',4096],"
                "['IrqLine',3],"
                "['IrqControl','irqctrl0'],"
                "['ClkSource','core0']"
                "]}]},"
    "{'Class':'PNPClass','Instances':["
          "{'Name':'pnp0','Attr':["
                "['LogLevel',4],"
                "['BaseAddress',0xfffff000],"
                "['Length',4096],"
                "['Tech',0],"
                "['AdcDetector',0xff]"
                "]}]},"
    "{'Class':'GrethClass','Instances':["
          "{'Name':'greth0','Attr':["
                "['LogLevel',1],"
                "['BaseAddress',0x80040000],"
                "['Length',0x40000],"
                "['IrqLine',2],"
                "['IrqControl','irqctrl0'],"
                "['IP',0x55667788],"
                "['MAC',0xfeedface00],"
                "['Bus','axi0'],"
                "['Transport','udpboard']"
                "]}]},"
    "{'Class':'BusClass','Instances':["
          "{'Name':'axi0','Attr':["
                "['LogLevel',3],"
                "['MapList',['bootrom0','fwimage0','sram0','gpio0',"
                        "'uart0','irqctrl0','gnss0','gptmr0',"
                        "'pnp0','dsu0','greth0']]"
                "]}]},"
    "{'Class':'GuiPluginClass','Instances':["
          "{'Name':'gui0','Attr':["
                "['LogLevel',4]"
                "]}]},"
    "{'Class':'BoardSimClass','Instances':["
          "{'Name':'boardsim','Attr':["
                "['LogLevel',1]"
                "]}]}"
  "]"
"}";

static char cfgbuf[1<<16];
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
    int cfgsz = 0;
    char path[1024];
    bool loadConfig = true;
    bool disableSim = true;
    AttributeType scriptFile("");

    // Parse arguments:
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-nocfg") == 0) {
                loadConfig = false;
            } else if (strcmp(argv[i], "-sim") == 0) {
                disableSim = false;
            } else if (strcmp(argv[i], "-script") == 0) {
                i++;
                scriptFile.make_string(argv[i]);
            }
        }
    }

    // Select configuration input (default/stored file):
    RISCV_init();
    RISCV_get_core_folder(path, sizeof(path));
    std::string cfg_filename = std::string(path) 
                    + std::string(JSON_CONFIG_FILE);

    if (loadConfig) {
        cfgsz = RISCV_read_json_file(cfg_filename.c_str(), cfgbuf, 
                                    static_cast<int>(sizeof(cfgbuf)));
    }

    // Convert string to attribute:
    if (cfgsz == 0) {
        Config.from_config(default_config);
    } else {
        Config.from_config(cfgbuf);
    }

    // Enable/Disable simulator option:
    Config["GlobalSettings"]["SimEnable"].make_boolean(!disableSim);
    //scriptFile.make_string("");
    Config["GlobalSettings"]["ScriptFile"] = scriptFile;

    RISCV_set_configuration(&Config);
   
    // Connect simulator to the EDCL debugger if enabled:
    if (!disableSim) {
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
        IFace *simple = RISCV_get_class("SimplePluginClass");
        if (simple) {
	        itst = static_cast<IService *>(RISCV_create_service(
                          simple, "example1", NULL));
        }
    }

    if (itst != NULL) {
        /** Get plugin specific interface. */
        ISimplePlugin * itst_access = static_cast<ISimplePlugin *>(
                                itst->getInterface(IFACE_SIMPLE_PLUGIN));
        /** Call example method */
        itst_access->exampleAction(0xcafe);
    }

    // Working cycle with console:
    IThread *in = static_cast<IThread *>(
        RISCV_get_service_iface("console0", IFACE_THREAD));
    if (in) {
        while (in->isEnabled()) {
            RISCV_sleep_ms(100);
        }
    }

    const char *t1 = RISCV_get_configuration();
    RISCV_write_json_file(cfg_filename.c_str(), t1);

    RISCV_cleanup();
	return 0;
}
