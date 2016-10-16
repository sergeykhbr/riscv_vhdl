/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      COM-port class implementation.
 */

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "api_types.h"
#include "api_core.h"
#include "coreservices/iserial.h"
#include "coreservices/isignal.h"
#include "comport.h"


namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(ComPortService)

ComPortService::ComPortService(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<ISerial *>(this));
    registerInterface(static_cast<IRawListener *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("UartSim", &uartSim_);
    registerAttribute("ComPortName", &comPortName_);
    registerAttribute("ComPortSpeed", &comPortSpeed_);

    isEnable_.make_boolean(true);
    uartSim_.make_string("");
    comPortName_.make_string("");
    comPortSpeed_.make_int64(115200);
    portListeners_.make_list(0);
    iuartSim_ = 0;
}

ComPortService::~ComPortService() {
}

void ComPortService::postinitService() {
    const AttributeType *glb = RISCV_get_global_settings();
    isSimulation_ = (*glb)["SimEnable"].to_bool();

    if (isSimulation_) {
        iuartSim_ = static_cast<ISerial *>(
            RISCV_get_service_iface(uartSim_.to_string(), IFACE_SERIAL));
        if (!iuartSim_) {
            RISCV_error("Can't get serial interface of UART simulator",
                        uartSim_.to_string());
            return;
        } else {
            iuartSim_->registerRawListener(static_cast<IRawListener *>(this));
        }
    } else {
        int err = openSerialPort(comPortName_.to_string(), 
            comPortSpeed_.to_int(), &hPort_);
        if (err < 0) {
            RISCV_error("Openning %s at %d . . .failed",
                        comPortName_.to_string(), comPortSpeed_.to_int());
            return;
        }
    }

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }
}

void ComPortService::predeleteService() {
    stop();
}

void ComPortService::busyLoop() {
    char tbuf[4096];
    int tbuf_cnt;
    while (isEnabled()) {
        // Sending...
        tbuf_cnt = 0;
        while (!txFifo_.isEmpty()) {
            tbuf[tbuf_cnt++] = txFifo_.get();

        }
        if (tbuf_cnt) {
            if (isSimulation_ && iuartSim_) {
                iuartSim_->writeData(tbuf, tbuf_cnt);
            } else if (!isSimulation_ && hPort_) {
                writeSerialPort(&hPort_, tbuf, tbuf_cnt);
            }
        }

        // Receiveing...
        if (!isSimulation_ && hPort_) {
            tbuf_cnt = readSerialPort(&hPort_, tbuf, tbuf_cnt);
        } else if (isSimulation_) {
            tbuf_cnt = 0;
            while (!rxFifo_.isEmpty()
                && tbuf_cnt < static_cast<int>(sizeof(tbuf))) {
                tbuf[tbuf_cnt++] = rxFifo_.get();
                tbuf[tbuf_cnt] = '\0';
            }
        }
        if (tbuf_cnt) {
            for (unsigned i = 0; i < portListeners_.size(); i++) {
                IRawListener *ilstn = static_cast<IRawListener *>(
                                portListeners_[i].to_iface());
                ilstn->updateData(tbuf, tbuf_cnt);
            }
        }
        RISCV_sleep_ms(50);
    }
    loopEnable_ = false;
    threadInit_.Handle = 0;
}

int ComPortService::writeData(const char *buf, int sz) {
    // @todo: mutex
    for (int i = 0; i < sz; i++) {
        txFifo_.put(buf[i]);
    }
    return sz;
}


void ComPortService::registerRawListener(IFace *iface) {
    AttributeType t1(iface);
    portListeners_.add_to_list(&t1);
}

void ComPortService::updateData(const char *buf, int buflen) {
    // Data from UART simulation:
    for (int i = 0; i < buflen; i++) {
        rxFifo_.put(buf[i]);
    }
}

}  // namespace debugger
