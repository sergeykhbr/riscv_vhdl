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

#include "RegWidget.h"
#include "RegsViewWidget.h"
#include "moc_RegsViewWidget.h"

#include <memory>

namespace debugger {

RegsViewWidget::RegsViewWidget(IGui *igui, QWidget *parent) 
    : QWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);
    cmdRegs_.make_string("regs");
    waitingResp_ = false;

    const AttributeType &cfg = (*igui_->getpConfig())["RegsViewWidget"];
    if (!cfg.is_dict()) {
        return;
    }
    const AttributeType &reglist = cfg["RegList"];
    const AttributeType &regwidth = cfg["RegWidthBytes"];

    if (!reglist.is_list()) {
        return;
    }

    for (unsigned row = 0; row < reglist.size(); row++) {
        const AttributeType &rowcfg = reglist[row];
        for (unsigned col = 0; col < rowcfg.size(); col++) {
            const AttributeType &regname = rowcfg[col];
            if (regname.size() == 0) {
                continue;
            }
            addRegWidget(row, col, regwidth.to_int(), regname.to_string());
        }
    }
    gridLayout->setColumnStretch(2*reglist.size() + 1, 10);
}

RegsViewWidget::~RegsViewWidget() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void RegsViewWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    resp_ = *resp;
    emit signalHandleResponse(&resp_);
    waitingResp_ = false;
}

void RegsViewWidget::slotUpdateByTimer() {
    if (!isVisible()) {
        return;
    }
    if (waitingResp_) {
        return;
    }
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                            &cmdRegs_, true);
    waitingResp_ = true;
}

void RegsViewWidget::slotRegChanged(AttributeType *wrcmd) {
    igui_->registerCommand(0, wrcmd, true);
}

void RegsViewWidget::addRegWidget(int row, int col, int bytes,
                                  const char *name) {
    QWidget *pnew = new RegWidget(name, bytes, this);
    gridLayout->addWidget(pnew, row + 1, col);

    connect(this, SIGNAL(signalHandleResponse(AttributeType *)),
            pnew, SLOT(slotHandleResponse(AttributeType *)));

    connect(pnew, SIGNAL(signalChanged(AttributeType *)),
            this, SLOT(slotRegChanged(AttributeType *)));
}

}  // namespace debugger
