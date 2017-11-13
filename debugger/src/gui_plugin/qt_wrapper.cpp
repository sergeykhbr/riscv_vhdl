/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      QT Wrapper connects QT libs to debugger core library.
 */

#include "api_core.h"
#include "qt_wrapper.h"
#include "moc_qt_wrapper.h"
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

void QtWrapper::slotMainWindowAboutToClose() {
    if (exiting_) {
        return;
    }
    /** Exit from GUI button push */
    exiting_ = true;
    RISCV_break_simulation();
}

}  // namespace debugger
