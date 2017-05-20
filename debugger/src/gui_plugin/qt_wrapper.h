/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      QT Wrapper connects QT libs to debugger core library.
 */
#ifndef __DEBUGGER_UI_QTHREAD_H__
#define __DEBUGGER_UI_QTHREAD_H__

#include "iclass.h"
#include "iservice.h"
#include "igui.h"
#include "MainWindow/DbgMainWindow.h"

namespace debugger {

class QtWrapper : public QObject {
    Q_OBJECT
public:
    QtWrapper(IGui *igui);
    virtual ~QtWrapper();

    void postInit(AttributeType *gui_cfg);
    void eventsUpdate();
    void pollingUpdate();
    void gracefulClose();

signals:
    void signalPollingUpdate();

private slots:
    void slotMainWindowAboutToClose();
    void slotAppDestroyed(QObject *obj);

private:
    IGui *igui_;
    QApplication *app_;
    DbgMainWindow *mainWindow_;
    event_def eventAppDestroyed_;
    bool first_start_;
    bool is_graceful_closing_;
};

}  // namespace debugger

#endif  // __DEBUGGER_UI_QTHREAD_H__
