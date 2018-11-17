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
    void gracefulClose();

    void externalCommand(AttributeType *req);

 private slots:
    void slotMainWindowAboutToClose();

 signals:
    void signalExternalCommand(const char *name);

 private:
    char extRequest_[16*1024];
    char *pextRequest_;
    IGui *igui_;
    DbgMainWindow *mainWindow_;
    bool exiting_;
};

}  // namespace debugger

#endif  // __DEBUGGER_UI_QTHREAD_H__
