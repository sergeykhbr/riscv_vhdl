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

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtCore/QEvent>
#include <QtGui/qevent.h>

namespace debugger {

class RegSetView : public QWidget,
                       public IGuiCmdHandler {
    Q_OBJECT
 public:
    RegSetView(IGui *igui, QWidget *parent, int cpucontext);
    virtual ~RegSetView();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

 signals:
    void signalHandleResponse(AttributeType *resp);
    void signalContextSwitchConfirmed();

 private slots:
    void slotUpdateByTimer();
    void slotHandleResponse(AttributeType *resp);
    void slotRegChanged(const char *wrcmd);
    void slotContextSwitchRequest(int idx);
    void slotContextSwitchConfirmed();

 private:
    void addRegWidget(int row, int col, int bytes, const char *name);

 private:
    //AttributeType cmdRegs_;
    AttributeType listRegs_;
    AttributeType cmdReg_;
    AttributeType respReg_;
    AttributeType responseRegChanged_;
    AttributeType responseCpuContext_;
    QGridLayout *gridLayout;
    
    IGui *igui_;
    bool waitingResp_;
    bool contextSwitchInProgress_;
    int curContextIdx_;
};

}  // namespace debugger
