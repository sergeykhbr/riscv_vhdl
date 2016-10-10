#include "RegWidget.h"
#include "RegsViewWidget.h"
#include "moc_RegsViewWidget.h"

#include <memory>

namespace debugger {

//static const uint64_t BASE_ADDR_DSU_REGS = 0x80090200;

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

    setWindowTitle(tr("Registers"));
    setMinimumWidth(150);
    setMinimumHeight(100);

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);

    minSizeApplied_ = false;
}


void RegsViewWidget::slotPostInit(AttributeType *cfg) {
    int n = 0;
    while (strcmp(REG_NAMES_LAYOUT[n], "break")) {
        if (REG_NAMES_LAYOUT[0] == '\0') {
            n++;
            continue;
        }
        addRegWidget(n, REG_NAMES_LAYOUT[n]);
        n++;
    }
}

void RegsViewWidget::slotUpdateByTimer() {
    if (isVisible()) {
        emit signalUpdateByTimer();
        update();
    }
}

void RegsViewWidget::slotTargetStateChanged(bool running) {
}

void RegsViewWidget::addRegWidget(int idx, const char *name) {
    int line = idx / 3;
    int col = idx - 3 * line;

    QWidget *pnew = new RegWidget(name, igui_, this);
    gridLayout->addWidget(pnew, line + 1, col);

    connect(this, SIGNAL(signalUpdateByTimer()),
            pnew, SLOT(slotUpdateByTimer()));

    if (!minSizeApplied_) {
        minSizeApplied_ = true;
        emit signalResize(QSize(3 * (pnew->minimumWidth() + 10), 
                          12 * (pnew->minimumHeight() + 5)));
    }
}


}  // namespace debugger
