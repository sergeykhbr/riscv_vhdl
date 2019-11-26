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

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "api_types.h"
#include "api_core.h"
#include "coreservices/iserial.h"
#include "comport.h"


namespace debugger {

ComPortService::ComPortService(const char *name) 
    : IService(name) {
    registerInterface(static_cast<IThread *>(this));
    registerInterface(static_cast<ISerial *>(this));
    registerInterface(static_cast<IRawListener *>(this));
    registerAttribute("Enable", &isEnable_);
    registerAttribute("UartSim", &uartSim_);
    registerAttribute("LogFile", &logFile_);
    registerAttribute("ComPortName", &comPortName_);
    registerAttribute("ComPortSpeed", &comPortSpeed_);

    isEnable_.make_boolean(true);
    uartSim_.make_string("");
    logFile_.make_string("uart0.log");
    logfile_ = NULL;
    comPortName_.make_string("");
    comPortSpeed_.make_int64(115200);
    portListeners_.make_list(0);
    iuartSim_ = 0;
    portOpened_ = false;
    RISCV_mutex_init(&mutexListeners_);
    prtHandler_ = 0;
}

ComPortService::~ComPortService() {
    RISCV_mutex_destroy(&mutexListeners_);
    if (logfile_) {
        fclose(logfile_);
        logfile_ = NULL;
    }
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
    }

    if (isEnable_.to_bool()) {
        if (!run()) {
            RISCV_error("Can't create thread.", NULL);
            return;
        }
    }

    logfile_ = 0;
    if (logFile_.size()) {
        char tst[256];
        RISCV_sprintf(tst, sizeof(tst), "Can't open '%s' file",
                      logFile_.to_string());
        logfile_ = fopen(logFile_.to_string(), "w");
    }
}

void ComPortService::predeleteService() {
    if (isSimulation_ && iuartSim_) {
        iuartSim_->unregisterRawListener(static_cast<IRawListener *>(this));
    }
}

//#define READ_RAWDATA_FROM_FILE
void ComPortService::busyLoop() {
    char tbuf[4096];
    int tbuf_cnt;
#ifdef READ_RAWDATA_FROM_FILE
    FILE *script = fopen("e:/uart0.log", "r");
    char logbuf[1024];
    int logbuf_sz;
#endif

    while (isEnabled()) {
#ifdef READ_RAWDATA_FROM_FILE
        logbuf_sz = fread(logbuf, 1, sizeof(logbuf), script);
        if (logbuf_sz) {
            for (unsigned i = 0; i < portListeners_.size(); i++) {
                IRawListener *ilstn = static_cast<IRawListener *>(
                                portListeners_[i].to_iface());
                ilstn->updateData(logbuf, logbuf_sz);
            }
        } else {
            fseek(script, 0, SEEK_SET);
        }
#endif
        if (!isSimulation_ && !portOpened_) {
            AttributeType settings;
            settings.make_list(4);
            settings[0u].make_int64(comPortSpeed_.to_int());
            int err = openPort(comPortName_.to_string(), settings);
            if (err < 0) {
                RISCV_error("Openning %s at %d . . .failed",
                        comPortName_.to_string(), comPortSpeed_.to_int());
                RISCV_sleep_ms(1000);
                continue;
            } else {
                portOpened_ = true;
            }
        }
        // Sending...
        tbuf_cnt = 0;
        while (!txFifo_.isEmpty()) {
            tbuf[tbuf_cnt++] = txFifo_.get();
        }
        if (tbuf_cnt) {
            if (isSimulation_ && iuartSim_) {
                iuartSim_->writeData(tbuf, tbuf_cnt);
            } else if (!isSimulation_ && prtHandler_) {
                writeSerialPort(&prtHandler_, tbuf, tbuf_cnt);
            }
        }

        // Receiveing...
        if (!isSimulation_ && prtHandler_) {
            tbuf_cnt = readSerialPort(&prtHandler_, tbuf,
                                        sizeof(tbuf) - tbuf_cnt);
            if (tbuf_cnt < 0) { 
                closePort();
                portOpened_ = false;
                continue;
            }
        } else if (isSimulation_) {
            tbuf_cnt = 0;
            while (!rxFifo_.isEmpty()
                && tbuf_cnt < static_cast<int>(sizeof(tbuf) - 1)) {
                tbuf[tbuf_cnt++] = rxFifo_.get();
                tbuf[tbuf_cnt] = '\0';
            }
        }
        if (tbuf_cnt) {
            RISCV_mutex_lock(&mutexListeners_);
            for (unsigned i = 0; i < portListeners_.size(); i++) {
                IRawListener *ilstn = static_cast<IRawListener *>(
                                portListeners_[i].to_iface());
                ilstn->updateData(tbuf, tbuf_cnt);
            }
            RISCV_mutex_unlock(&mutexListeners_);
            if (logfile_) {
                fwrite(tbuf, tbuf_cnt, 1, logfile_);
                fflush(logfile_);
            }
        }

        RISCV_sleep_ms(50);
    }
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
    RISCV_mutex_lock(&mutexListeners_);
    portListeners_.add_to_list(&t1);
    RISCV_mutex_unlock(&mutexListeners_);
}

void ComPortService::unregisterRawListener(IFace *iface) {
    RISCV_mutex_lock(&mutexListeners_);
    for (unsigned i = 0; i < portListeners_.size(); i++) {
        IFace *itmp = portListeners_[i].to_iface();
        if (itmp == iface) {
            portListeners_.remove_from_list(i);
            break;
        }
    }
    RISCV_mutex_unlock(&mutexListeners_);
}

int ComPortService::updateData(const char *buf, int buflen) {
    // Data from UART simulation:
    for (int i = 0; i < buflen; i++) {
        rxFifo_.put(buf[i]);
    }
    return buflen;
}

}  // namespace debugger
