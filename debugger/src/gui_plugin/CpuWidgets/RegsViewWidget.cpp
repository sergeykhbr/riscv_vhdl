/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Single CPU register form.
 */

#include "RegWidget.h"
#include "RegsViewWidget.h"
#include "moc_RegsViewWidget.h"

#include <memory>

namespace debugger {

/** Layout of register by name */
static const char *REG_NAMES_LAYOUT[] = {
    "ra", "s0",  "a0",
    "sp", "s1",  "a1",
    "gp", "s2",  "a2",
    "tp", "s3",  "a3",
    ""  , "s4",  "a4",
    "t0", "s5",  "a5",
    "t1", "s6",  "a6",
    "t2", "s7",  "a7",
    "t3", "s8",  "",
    "t4", "s9",  "",
    "t5", "s10", "pc",
    "t6", "s11", "npc",
    "break"
};

RegsViewWidget::RegsViewWidget(IGui *igui, QWidget *parent) 
    : UnclosableWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);
    cmdRegs_.make_string("regs");
    waitingResp_ = false;

    int n = 0;
    while (strcmp(REG_NAMES_LAYOUT[n], "break")) {
        if (REG_NAMES_LAYOUT[n][0] == '\0') {
            n++;
            continue;
        }
        addRegWidget(n, REG_NAMES_LAYOUT[n]);
        n++;
    }
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

void RegsViewWidget::addRegWidget(int idx, const char *name) {
    int line = idx / 3;
    int col = idx - 3 * line;

    QWidget *pnew = new RegWidget(name, this);
    gridLayout->addWidget(pnew, line + 1, col);

    connect(this, SIGNAL(signalHandleResponse(AttributeType *)),
            pnew, SLOT(slotHandleResponse(AttributeType *)));

    connect(pnew, SIGNAL(signalChanged(AttributeType *)),
            this, SLOT(slotRegChanged(AttributeType *)));
}

}  // namespace debugger
