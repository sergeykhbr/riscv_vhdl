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

#include "RegSetView.h"
#include "RegWidget.h"
#include <QtWidgets/QLabel>

#include <memory>

namespace debugger {

RegSetView::RegSetView(IGui *igui, QWidget *parent, int cpucontext) 
    : QWidget(parent) {
    igui_ = igui;
    curContextIdx_ = cpucontext;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);
    waitingResp_ = false;
    contextSwitchInProgress_ = false;

    const AttributeType &cfg = (*igui_->getpConfig())["RegsViewWidget"];
    if (!cfg.is_dict()) {
        return;
    }
    const AttributeType &cpuctx = cfg["CpuContext"][cpucontext];
    if (!cpuctx.is_dict()) {
        return;
    }

    int regsetidx = cpuctx["RegisterSetIndex"].to_int();
    const AttributeType &regset = cfg["RegisterSet"][regsetidx];
    if (!regset.is_dict()) {
        return;
    }
    const AttributeType &reglist = regset["RegList"];
    const AttributeType &regwidth = regset["RegWidthBytes"];

    if (!reglist.is_list()) {
        return;
    }

    QString qstrReg = tr("reg ");

    for (unsigned row = 0; row < reglist.size(); row++) {
        const AttributeType &rowcfg = reglist[row];
        for (unsigned col = 0; col < rowcfg.size(); col++) {
            const AttributeType &regname = rowcfg[col];
            if (regname.size() == 0) {
                continue;
            }
            qstrReg += QString(regname.to_string()) + tr(" ");
            addRegWidget(row, col, regwidth.to_int(), regname.to_string());
        }
    }
    cmdReg_.make_string(qstrReg.toLatin1());

    gridLayout->setColumnStretch(2*reglist.size() + 1, 10);
    connect(this, SIGNAL(signalHandleResponse(AttributeType *)),
                  SLOT(slotHandleResponse(AttributeType *)));

    connect(this, SIGNAL(signalContextSwitchConfirmed()),
                  SLOT(slotContextSwitchConfirmed()));
}

RegSetView::~RegSetView() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void RegSetView::handleResponse(const char *cmd) {
    if (strstr(cmd, "cpucontext") != 0) {
        emit signalContextSwitchConfirmed();
        return;
    }
    emit signalHandleResponse(&respReg_);
}

void RegSetView::slotHandleResponse(AttributeType *resp) {
    // To avoid resp_ overwiting before register views udpated:
    waitingResp_ = false;
}

void RegSetView::slotUpdateByTimer() {
    if (!isVisible()) {
        return;
    }
    if (waitingResp_) {
        return;
    }
    if (contextSwitchInProgress_) {
        return;
    }
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           cmdReg_.to_string(), &respReg_, true);
    waitingResp_ = true;
}

void RegSetView::slotRegChanged(const char *wrcmd) {
    igui_->registerCommand(0, wrcmd, &responseRegChanged_, true);
}

void RegSetView::slotContextSwitchRequest(int idx) {
    char tstr[64];
    RISCV_sprintf(tstr, sizeof(tstr), "cpucontext %d", idx);
    contextSwitchInProgress_ = true;
    igui_->registerCommand(0, tstr, &responseRegChanged_, true);
    curContextIdx_ = idx;
}

void RegSetView::slotContextSwitchConfirmed() {
    contextSwitchInProgress_ = false;
}

void RegSetView::addRegWidget(int row, int col, int bytes,
                                  const char *name) {
    QLabel *label = new QLabel(this);
    QWidget *pnew;
    pnew = new RegWidget(name, bytes, this);
    label->setText(tr(name));
    /*
    QSizePolicy labelSizePolicy(QSizePolicy::Preferred, 
                                QSizePolicy::Preferred);
    labelSizePolicy.setHorizontalStretch(0);
    labelSizePolicy.setVerticalStretch(0);
    labelSizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(labelSizePolicy);
    label->setText(name_);
    label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
*/

    gridLayout->addWidget(label, row + 1, 2*col, Qt::AlignLeft);
    gridLayout->addWidget(pnew, row + 1, 2*col + 1, Qt::AlignLeft);

    connect(this, SIGNAL(signalHandleResponse(AttributeType *)),
            pnew, SLOT(slotHandleResponse(AttributeType *)));

    connect(pnew, SIGNAL(signalChanged(const char *)),
            this, SLOT(slotRegChanged(const char *)));
}

}  // namespace debugger
