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
 *
 * @brief      Serial console emulator.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iserial.h"

#include "UartEditor.h"
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtCore/QEvent>

namespace debugger {

class UartWidget : public QWidget {
    Q_OBJECT
public:
    UartWidget(IGui *igui, QWidget *parent);

private:
    UartEditor *editor_;
};

class UartQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    UartQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                      QAction *act = 0)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("uart0"));
        setMinimumWidth(parent->size().width() / 2);
        QWidget *pnew = new UartWidget(igui, this);
        setWindowIcon(QIcon(tr(":/images/serial_96x96.png")));
        if (act) {
            act->setChecked(true);
        }
        setWidget(pnew);
        area_->addSubWindow(this);
        show();
    }
    
protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE {
        if (action_) {
            action_->setChecked(false);
        }
        area_->removeSubWindow(this);
        event_->accept();
    }
private:
    QAction *action_;
    QMdiArea *area_;
};

}  // namespace debugger
