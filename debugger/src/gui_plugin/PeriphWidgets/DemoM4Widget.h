/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtCore/QEvent>
#include <QtGui/qevent.h>

namespace debugger {

class DemoM4Widget : public QWidget {
    Q_OBJECT
 public:
    DemoM4Widget(IGui *igui, QWidget *parent);

 signals:
    void signalUpdateByTimer();

 private slots:
    void slotUpdateByTimer();

 private:
    QGridLayout *gridLayout;
    
    IGui *igui_;
};

class DemoM4QMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
 public:
    DemoM4QMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                        QAction *act = 0) : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("STM32L4xx Platform Demo"));
        QWidget *pnew = new DemoM4Widget(igui, this);
        if (act) {
            setWindowIcon(act->icon());
            act->setChecked(true);
            connect(parent, SIGNAL(signalUpdateByTimer()),
                    pnew, SLOT(slotUpdateByTimer()));
        }
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Qt::white);
        setPalette(pal);

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
