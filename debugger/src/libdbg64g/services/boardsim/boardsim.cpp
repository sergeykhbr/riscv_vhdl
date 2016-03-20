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
    registerAttribute("Transport", &transport_);
    registerAttribute("Disable", &isDisable_);

    isDisable_.make_boolean(true);
    transport_.make_string("");
}

void BoardSim::postinitService() {
    IService *iserv = 
        static_cast<IService *>(RISCV_get_service(transport_.to_string()));
    if (!iserv) {
        RISCV_error("Transport service '%'s not found", 
                    transport_.to_string());
    }
    itransport_ = static_cast<IUdp *>(iserv->getInterface(IFACE_UDP));
    if (itransport_) {
        itransport_->registerListener(static_cast<IRawListener *>(this));
    } else {
        RISCV_error("UDP interface '%s' not found", 
                    transport_.to_string());
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

thread_return_t BoardSim::runThread(void *arg) {
    ((BoardSim*)arg)->busyLoop();
    return 0;
}

void BoardSim::busyLoop() {
    int bytes;
    //char msg[] = {0x00, 0x00, 0x00, 0x18, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};
    char msg[] = {0x04, 0x40, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    while (loopEnable_) {
        bytes = itransport_->readData(rxbuf, static_cast<int>(sizeof(rxbuf)));

        if (bytes != 0) {
            itransport_->sendData(msg, sizeof(msg));
        }
    }
}

}  // namespace debugger

