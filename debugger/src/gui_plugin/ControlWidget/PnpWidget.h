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
#include "../../socsim_plugin/periphmap.h"
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtWidgets/QLabel>
#include <QtCore/QEvent>
#include <QtGui/qevent.h>

namespace debugger {

class PnpWidget : public QWidget,
                  public IGuiCmdHandler {
    Q_OBJECT
public:
    PnpWidget(IGui *igui, QWidget *parent);
    virtual ~PnpWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

signals:
    void signalUpdate();

private slots:
    void slotUpdate();

protected:
    virtual void showEvent(QShowEvent *event_);
    QLabel *getLabel(int id) {
        return static_cast<QLabel *>(mainLayout_->itemAt(id)->widget());
    }

private:
    QWidget *parent_;
    IGui *igui_;
    QGridLayout *mainLayout_;
    
    AttributeType reqcmd_;
    AttributeType respcmd_;
    AttributeType lstSlaves_;
    enum ETargets {SIMULATION, ML605, KC705, TARGET_Unknown, TARGETS_total};
    QImage imgTarget_[TARGETS_total];

    PnpMapType pnp_;
    union DescriptorTableType {
        DeviceDescriptorType *item;
        uint8_t *buf;
    } iter_;
};

class PnpQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    PnpQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                      QAction *act)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("Plug'n'Play info"));
        setWindowIcon(QIcon(tr(":/images/board_96x96.png")));
        QWidget *pnew = new PnpWidget(igui, this);
        act->setChecked(true);
        setWidget(pnew);
        area_->addSubWindow(this);
        show();
    }
    
protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE {
        action_->setChecked(false);
        area_->removeSubWindow(this);
        event_->accept();
    }
private:
    QAction *action_;
    QMdiArea *area_;
};

}  // namespace debugger
