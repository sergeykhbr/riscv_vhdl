#include "RegWidget.h"
#include "RegsViewWidget.h"
#include "moc_RegsViewWidget.h"

#include <memory>
#include <QtWidgets/qgridlayout.h>

namespace debugger {

static const char *REG_NAMES[] = {
    "ra ", "s0 ", "a0 ",
    "sp ", "s1 ", "a1 ",
    "gp ", "s2 ", "a2 ",
    "tp ", "s3 ", "a3 ",
    ""   , "s4 ", "a4 ",
    "t0 ", "s5 ", "a5 ",
    "t1 ", "s6 ", "a6 ",
    "t2 ", "s7 ", "a7 ",
    "t3 ", "s8 ", "",
    "t4 ", "s9 ", "",
    "t5 ", "s10", "pc ",
    "t6 ", "s11", "npc"
};

RegsViewWidget::RegsViewWidget(IGui *igui, QWidget *parent) 
    : QWidget(parent) {
    igui_ = igui;

    setWindowTitle(tr("Registers"));
    setMinimumWidth(150);
    setMinimumHeight(100);

    pollingTimer_ = new QTimer(this);
    pollingTimer_->setSingleShot(false);
    connect(pollingTimer_, SIGNAL(timeout()), this, SLOT(slotPollingUpdate()));

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);

    QWidget *pnew;
    for (int i = 0; i < 12; i++) {
        if (REG_NAMES[3*i + 0][0] != '\0') {
            pnew = new RegWidget(REG_NAMES[3*i + 0], this);
            gridLayout->addWidget(pnew, i + 1, 0);
        }

        if (REG_NAMES[3*i + 1][0] != '\0') {
            pnew = new RegWidget(REG_NAMES[3*i + 1], this);
            gridLayout->addWidget(pnew, i + 1, 1);
        }

        if (REG_NAMES[3*i + 2][0] != '\0') {
            pnew = new RegWidget(REG_NAMES[3*i + 2], this);
            gridLayout->addWidget(pnew, i + 1, 2);
        }
    }
}


void RegsViewWidget::handleResponse(AttributeType *req, AttributeType *resp) {
}

void RegsViewWidget::slotConfigure(AttributeType *cfg) {
    for (unsigned i = 0; i < cfg->size(); i++) {
        const AttributeType &attr = (*cfg)[i];
        if (strcmp(attr[0u].to_string(), "PollingMs") == 0) {
            int ms = static_cast<int>(attr[1].to_uint64());
            if (ms != 0) {
                pollingTimer_->start(attr[1].to_uint64());
            }
        }
    }
}

void RegsViewWidget::slotPollingUpdate() {
}

}  // namespace debugger
