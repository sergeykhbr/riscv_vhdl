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
    "'GUI':false,"
    "'ScriptFile':''"
  "},"
  "'Services':["
    "{'Class':'GuiPluginClass','Instances':["
                "{'Name':'gui0','Attr':["
                "['LogLevel',4],"
                "['WidgetsConfig',{"
                  "'Serial':'port1',"
                  "'AutoComplete':'autocmd0',"
                  "'SocInfo':'info0',"
                  "'PollingMs':250"
                "}],"
                "['SocInfo','info0'],"
                "['CommandExecutor','cmdexec0']"
                "]}]},"
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
    "{'Class':'ComPortServiceClass','Instances':["
          "{'Name':'port1','Attr':["
                "['LogLevel',1],"
                "['Enable',true],"
                "['UartSim','uart0'],"
                "['ComPortName','COM3'],"
                "['ComPortSpeed',115200]]}]},"
    "{'Class':'ElfLoaderServiceClass','Instances':["
          "{'Name':'loader0','Attr':["
                "['LogLevel',4],"
                "['Tap','edcltap'],"
                "['VerifyEna',true]]}]},"
    "{'Class':'ConsoleServiceClass','Instances':["
          "{'Name':'console0','Attr':["
                "['LogLevel',4],"
                "['Enable',true],"
                "['StepQueue','core0'],"
                "['AutoComplete','autocmd0'],"
                "['CommandExecutor','cmdexec0'],"
                "['Signals','gpio0'],"
                "['InputPort','port1']]}]},"
    "{'Class':'AutoCompleterClass','Instances':["
          "{'Name':'autocmd0','Attr':["
                "['LogLevel',4],"
                "['SocInfo','info0']"
                "['HistorySize',64],"
                "['History',["
                     "'csr MCPUID',"
                     "'csr MTIME',"
                     "'read 0xfffff004 128',"
                     "'loadelf helloworld'"
                     "]]"
                "]}]},"
    "{'Class':'CmdExecutorClass','Instances':["
          "{'Name':'cmdexec0','Attr':["
                "['LogLevel',4],"
                "['Tap','edcltap'],"
                "['SocInfo','info0']"
                "]}]},"
    "{'Class':'SocInfoClass','Instances':["
          "{'Name':'info0','Attr':["
                "['LogLevel',4],"
                "['PnpBaseAddress',0xFFFFF000],"
                "['GpioBaseAddress',0x80000000],"
                "['DsuBaseAddress',0x80080000],"
                "['ListRegs',[['zero',8,0],['ra',8,1],['sp',8,2],['gp',8,3],"
                            "['tp',8,4],['t0',8,5],['t1',8,6],['t2',8,7],"
                            "['s0',8,8],['s1',8,9],['a0',8,10],['a1',8,11],"
                            "['a2',8,12],['a3',8,13],['a4',8,14],['a5',8,15],"
                            "['a6',8,16],['a7',8,17],['s2',8,18],['s3',8,19],"
                            "['s4',8,20],['s5',8,21],['s6',8,22],['s7',8,23],"
                            "['s8',8,24],['s9',8,25],['s10',8,26],['s11',8,27],"
                            "['t3',8,28],['t4',8,29],['t5',8,30],['t6',8,31],"
                            "['pc',8,32,'Instruction Pointer'],"
                            "['npc',8,33,'Next IP']]],"
                "['ListCSR',["
                    "['MCPUID',8,0xf00,'CPU description'],"
                    "['MRESET',8,0x782],"
                    "['MTIME',8,0x701,'Machine wall-clock time.'],"
                    "['MEPC',8,0x341,'Machine exception program counter']"
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
    "{'Class':'BoardSimClass','Instances':["
          "{'Name':'boardsim','Attr':["
                "['LogLevel',1]"
                "]}]}"
  "]"
"}";

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
    char path[1024];
    bool loadConfig = true;
    bool disableSim = true;
    bool disableGui = true;
    AttributeType scriptFile("");

    // Parse arguments:
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-nocfg") == 0) {
                loadConfig = false;
            } else if (strcmp(argv[i], "-sim") == 0) {
                disableSim = false;
            } else if (strcmp(argv[i], "-gui") == 0) {
                disableGui = false;
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

    AttributeType databuf;
    if (loadConfig) {
        RISCV_read_json_file(cfg_filename.c_str(), &databuf);
    } 
    if (databuf.size() == 0) {
        databuf.make_string(default_config);
    }

    Config.from_config(databuf.to_string());

    // Enable/Disable simulator option:
    Config["GlobalSettings"]["SimEnable"].make_boolean(!disableSim);
    Config["GlobalSettings"]["GUI"].make_boolean(!disableGui);
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
