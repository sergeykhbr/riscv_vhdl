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
#include "coreservices/iautocomplete.h"
#include "coreservices/irawlistener.h"
#include "coreservices/icmdexec.h"

#include <QtWidgets/QPlainTextEdit>
#include <QtGui/qevent.h>

namespace debugger {

/**
 * QPlainTextEdit gives per-line scrolling (but optimized for plain text)
 * QTextEdit gives smooth scrolling (line partial move up-down)
 */
class ConsoleWidget : public QPlainTextEdit,
                      public IGuiCmdHandler,
                      public IRawListener {
    Q_OBJECT
 public:
    ConsoleWidget(IGui *igui, QWidget *parent = 0);
    ~ConsoleWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

    /** IRawListener */
    virtual int updateData(const char *buf, int buflen);

 signals:
    void signalNewData();
 private slots:
    void slotUpdateByData();

 protected:
    virtual void keyPressEvent(QKeyEvent *e);

 private:
    IGui *igui_;
    IAutoComplete *iauto_;

    AttributeType cursorPos_;
    AttributeType reqcmd_;
    AttributeType respcmd_;

    int cursorMinPos_;

    wchar_t *wcsConv_;
    char *mbsConv_;
    int sizeConv_;
    mutex_def mutexOutput_;
    QString strOutput_;
    QFont fontMainText_;
    QFont fontRISCV_;
};

}  // namespace debugger
