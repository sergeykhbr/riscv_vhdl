/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Simulator of the FPGA board with Ethernet UDP/EDCL interface.
 */

#include "api_core.h"
#include "boardsim.h"

namespace debugger {

/** Class registration in the Core */
static BoardSimClass local_class_;

BoardSim::BoardSim(const char *name)  : IService(name) {
    registerInterface(static_cast<IBoardSim *>(this));
    registerInterface(static_cast<IRawListener *>(this));
}

void BoardSim::postinitService() {
    if (!config_.is_dict()) {
        RISCV_error("Wrong configuration", NULL);
        return;
    }
    const char *transport_name = config_["Transport"].to_string();
    IService *iserv = 
        static_cast<IService *>(RISCV_get_service(transport_name));
    if (!iserv) {
        RISCV_error("Transport service '%'s not found", transport_name);
    }
    itransport_ = static_cast<IUdp *>(iserv->getInterface(IFACE_UDP));
    if (itransport_) {
        itransport_->registerListener(static_cast<IRawListener *>(this));
    } else {
        RISCV_error("UDP interface '%s' not found", transport_name);
    }
}

void BoardSim::deleteService() {
    stopSimulator();
    RISCV_thread_join(threadInit_.Handle, 50000);
}

void BoardSim::runSimulator() {
    loopEnable_ = false;
    threadInit_.func = (lib_thread_func)runThread;
    threadInit_.args = this;
    RISCV_thread_create(&threadInit_);

    if (threadInit_.Handle) {
        loopEnable_ = true;
        RISCV_info("UDP thread was run successfully", NULL);
    } else {
        RISCV_error("Can't create thread.", NULL);
    }
}

void BoardSim::stopSimulator() {
    loopEnable_ = false;
}

AttributeType BoardSim::getConnectionSettings() {
    AttributeType ret;
    if (config_["UseHW"].to_int64()) {
        ret.make_dict();
        ret["IP"] = config_["HW_IP"];
        // Port maybe any. EDCL ignores this value.
        ret["Port"] = AttributeType(Attr_UInteger, 5555ull);
    } else if (itransport_) {
        ret = itransport_->getConnectionSettings();
    } else {
        RISCV_error("Transport not defined", NULL);
    }
    return ret;
}

void BoardSim::setTargetSettings(const AttributeType *target) {
    if (!itransport_) {
        RISCV_error("Transport not defined", NULL);
        return;
    }
    itransport_->setTargetSettings(target);
}

thread_return_t BoardSim::runThread(void *arg) {
    ((BoardSim*)arg)->busyLoop();
    return 0;
}

void BoardSim::busyLoop() {
    int bytes;
    char msg[] = "ACK\n";

    while (loopEnable_) {
        bytes = itransport_->readData(rxbuf, static_cast<int>(sizeof(rxbuf)));

        if (bytes != 0) {
            itransport_->sendData(msg, 4);
        }
    }
}

}  // namespace debugger

