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

#include "api_core.h"
#include "qt_wrapper.h"
#include <string>
#include <QtWidgets/QApplication>

#define QT_EXEC_IMPL

namespace debugger {

static event_def eventAppDestroyed_;

/** It's a blocking event that runs only once */
void ui_events_update(void *args) {
    QtWrapper *ui = reinterpret_cast<QtWrapper *>(args);
    ui->eventsUpdate();
    RISCV_event_set(&eventAppDestroyed_);
    //RISCV_event_close(&eventAppDestroyed_);
}

QtWrapper::QtWrapper(IGui *igui) 
    : QObject() {
    igui_ = igui;
    exiting_ = false;
    pextRequest_ = extRequest_;
    RISCV_event_create(&eventAppDestroyed_, "eventAppDestroyed_");
}

QtWrapper::~QtWrapper() {
}

void QtWrapper::postInit(AttributeType *gui_cfg) {
    RISCV_register_timer(1, 1, ui_events_update, this);
}

void QtWrapper::eventsUpdate() {
    int argc = 0;
    char *argv[] = {0};

    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(true);

    mainWindow_ = new DbgMainWindow(igui_);
    connect(mainWindow_, SIGNAL(signalAboutToClose()), 
            this, SLOT(slotMainWindowAboutToClose()));

    mainWindow_->show();
    app.exec();
    RISCV_unregister_timer(ui_events_update);

    delete mainWindow_;
    app.quit();
}

void QtWrapper::gracefulClose() {
    if (!exiting_) {
        /** Exit through console command 'exit' */
        mainWindow_->close();
        RISCV_event_wait_ms(&eventAppDestroyed_, 10000);
    }
}

void QtWrapper::externalCommand(AttributeType *req) {
    size_t free_cnt = sizeof(extRequest_) - (pextRequest_ - extRequest_);
    if (free_cnt <= req->size()) {
        pextRequest_ = extRequest_;
    }
    memcpy(pextRequest_, req->to_string(), req->size() + 1);
    emit signalExternalCommand(pextRequest_);
    pextRequest_ += req->size() + 1;
}

void QtWrapper::slotMainWindowAboutToClose() {
    if (exiting_) {
        return;
    }
    /** Exit from GUI button push */
    exiting_ = true;
    RISCV_break_simulation();
}

}  // namespace debugger
