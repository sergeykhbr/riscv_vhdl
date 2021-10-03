/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "ButtonGenWidget.h"
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtGui/QAction>
#include <QtGui/QPainter>

namespace debugger {

static const int BTN_SZ_PIX = 32;

GenericButtonWidget::GenericButtonWidget(IGui *igui, QWidget *parent,
    const char *btnname, const char *picname) : QPushButton(parent) {
    name_.make_string(btnname);
    igui_ = igui;

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s press", btnname);
    cmd_press_.make_string(tstr);

    RISCV_sprintf(tstr, sizeof(tstr), "%s release", btnname);
    cmd_release_.make_string(tstr);

    QIcon *ico = new QIcon();
    QImage img(tr(picname));
    QImage img_inv(tr(picname));
    img_inv.invertPixels();
    
    ico->addPixmap(QPixmap::fromImage(img), QIcon::Normal, QIcon::Off);
    ico->addPixmap(QPixmap::fromImage(img_inv), QIcon::Normal, QIcon::On);
    setIcon(*ico);

    QSize bsz(BTN_SZ_PIX, BTN_SZ_PIX);
    setIconSize(bsz);
    setFixedSize(bsz);
    setFlat(false);
    setCheckable(true);

    QWidget *qgui = reinterpret_cast<QWidget *>(igui_->getQGui());
    connect(qgui, SIGNAL(signalExternalCommand(const char *)),
            this, SLOT(slotExternalCommand(const char *)));

    connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
    connect(this, SIGNAL(signalHandleResponse()), SLOT(slotHandleResponse()));
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           name_.to_string(), &response_, true);
}

GenericButtonWidget::~GenericButtonWidget() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void GenericButtonWidget::handleResponse(const char *cmd) {
    value_ = response_.to_bool();
    emit signalHandleResponse();
}

void GenericButtonWidget::slotToggled(bool val) {
    if (value_ == val) {
        return;
    }
    if (val) {
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                               cmd_press_.to_string(), &response_, true);
    } else {
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                               cmd_release_.to_string(), &response_, true);
    }
}

void GenericButtonWidget::slotHandleResponse() {
    if (value_) {
        setChecked(true);
    } else {
        setChecked(false);
    }
}

void GenericButtonWidget::slotExternalCommand(const char *cmd) {
    if (cmd_press_.is_equal(cmd)) {
        setChecked(value_ = true);
    } else if (cmd_release_.is_equal(cmd)) {
        setChecked(value_ = false);
    }
}

}  // namespace debugger
