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

namespace debugger {

void ui_events_update(void *args) {
    QtWrapper *ui = reinterpret_cast<QtWrapper *>(args);
    ui->eventsUpdate();
}

void ui_polling_update(void *args) {
    QtWrapper *ui = reinterpret_cast<QtWrapper *>(args);
    ui->pollingUpdate();
}

void ui_graceful_exit(void *args) {
    QtWrapper *ui = reinterpret_cast<QtWrapper *>(args);
    ui->gracefulClose();
}

QtWrapper::QtWrapper(IGui *igui) 
    : QObject() {
    igui_ = igui;
    mainWindow_ = 0;
    is_graceful_closing_ = false;
    RISCV_event_create(&eventAppDestroyed_, "eventAppDestroyed_");

    int argc = 0;
    char *argv[] = {0};

    app_ = new QApplication(argc, argv);
    app_->setQuitOnLastWindowClosed(false);

    connect(app_, SIGNAL(destroyed(QObject *)), 
            this, SLOT(slotAppDestroyed(QObject *)), Qt::DirectConnection);
}

QtWrapper::~QtWrapper() {
    is_graceful_closing_ = true;
    RISCV_unregister_timer(ui_polling_update);
    RISCV_unregister_timer(ui_events_update);
    RISCV_register_timer(1, 1, ui_graceful_exit, this);

    RISCV_event_wait_ms(&eventAppDestroyed_, 10000);
}

void QtWrapper::postInit(AttributeType *gui_cfg) {
    mainWindow_ = new DbgMainWindow(igui_);
    connect(this, SIGNAL(signalPollingUpdate()), 
            mainWindow_, SLOT(slotUpdateByTimer()));

    connect(mainWindow_, SIGNAL(signalAboutToClose()), 
            this, SLOT(slotMainWindowAboutToClose()));

    RISCV_register_timer((*gui_cfg)["PollingMs"].to_int(), 
                         0, ui_polling_update, this);
    RISCV_register_timer((*gui_cfg)["EventsLoopMs"].to_int(),
                         0, ui_events_update, this);
    first_start_ = true;
}

void QtWrapper::eventsUpdate() {
    if (first_start_) {
        first_start_ = false;
        mainWindow_->show();
    }
    app_->processEvents();
}

void QtWrapper::pollingUpdate() {
    emit signalPollingUpdate();
}

void QtWrapper::gracefulClose() {
    app_->closeAllWindows();
    app_->processEvents();
    delete mainWindow_;
    app_->processEvents();
    app_->quit();
    app_->processEvents();
    delete app_;
}

void QtWrapper::slotMainWindowAboutToClose() {
    if (is_graceful_closing_) {
        return;
    }
    RISCV_unregister_timer(ui_polling_update);
    RISCV_break_simulation();
}

void QtWrapper::slotAppDestroyed(QObject *obj) {
    RISCV_event_set(&eventAppDestroyed_);
}

}  // namespace debugger
