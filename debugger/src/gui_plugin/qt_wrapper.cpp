/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      UI in a separate QThread.
 */

#include "api_core.h"
#include "qt_wrapper.h"
#include "moc_qt_wrapper.h"
#include <string>
#include <QtWidgets/QApplication>

namespace debugger {

void ui_events_update(void *args) {
    QtWrapper *ui = reinterpret_cast<QtWrapper *>(args);
    ui->eventsUpdate();
}

void ui_polling_update(void *args) {
    QtWrapper *ui = reinterpret_cast<QtWrapper *>(args);
    ui->pollingUpdate();
}

QtWrapper::QtWrapper(IGui *igui) 
    : QObject() {
    igui_ = igui;
    //eventInitDone_ = init_done;
    mainWindow_ = 0;
    RISCV_event_create(&eventAppDestroyed_, "eventAppDestroyed_");
    //thread_ = new QThread();
    //connect(thread_, SIGNAL(started()),
    //        this, SLOT(slotOnStarted()),
    //        Qt::DirectConnection);
    //thread_->start();

    int argc = 0;
    char *argv[] = {0};

    app_ = new QApplication(argc, argv);
    app_->setQuitOnLastWindowClosed(true);

    connect(app_, SIGNAL(destroyed(QObject *)), 
            this, SLOT(slotAppDestroyed(QObject *)), Qt::DirectConnection);
}

QtWrapper::~QtWrapper() {
    RISCV_unregister_timer(ui_events_update);
    delete mainWindow_;
    qApp->processEvents();
    qApp->quit();
    if (qApp) {
        qApp->processEvents();
    }
    if (qApp) {
        delete qApp;
    }
    RISCV_event_wait_ms(&eventAppDestroyed_, 10000);
}

void QtWrapper::postInit(AttributeType *gui_cfg) {
    mainWindow_ = new DbgMainWindow(igui_);
    connect(this, SIGNAL(signalPollingUpdate()), 
            mainWindow_, SLOT(slotUpdateByTimer()));

    connect(mainWindow_, SIGNAL(signalAboutToClose()), 
            this, SLOT(slotMainWindowAboutToClose()));

    RISCV_register_timer(200, ui_polling_update, this);
    RISCV_register_timer(10, ui_events_update, this);

    mainWindow_->show();
}

void QtWrapper::eventsUpdate() {
    app_->processEvents();
}

void QtWrapper::pollingUpdate() {
    emit signalPollingUpdate();
}

void QtWrapper::slotMainWindowAboutToClose() {
    RISCV_unregister_timer(ui_polling_update);
    RISCV_break_simulation();
}

void QtWrapper::slotAppDestroyed(QObject *obj) {
    RISCV_event_set(&eventAppDestroyed_);;
}

}  // namespace debugger
