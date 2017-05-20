/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      QT Wrapper to connect it to debugger core library.
 */
#ifndef __DEBUGGER_UI_QTHREAD_H__
#define __DEBUGGER_UI_QTHREAD_H__

#include "iclass.h"
#include "iservice.h"
#include "igui.h"
#include "MainWindow/DbgMainWindow.h"

namespace debugger {

    /**
     * This UiThread and UiInitDone event allow us to register all widgets
     * interfaces before PostInit stage started and as results make them 
     * visible to all other plugins.
     */
class QtWrapper : public QObject {
    Q_OBJECT
public:
    QtWrapper(IGui *igui);
    virtual ~QtWrapper();

    void postInit(AttributeType *gui_cfg);
    void eventsUpdate();
    void pollingUpdate();

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
};

}  // namespace debugger

#endif  // __DEBUGGER_UI_QTHREAD_H__
