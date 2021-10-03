/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory editor form.
 */

#include "MemArea.h"
#include "MemControl.h"
#include "MemViewWidget.h"
#include <memory>

namespace debugger {

MemViewWidget::MemViewWidget(IGui *igui, QWidget *parent,
    uint64_t addr, uint64_t sz) : QWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);

    MemControl *pctrl = new MemControl(this, addr, sz);

    MemArea *parea = new MemArea(igui, this, addr, sz);
    gridLayout->addWidget(pctrl , 0, 0);
    gridLayout->addWidget(parea, 1, 0);
    gridLayout->setRowStretch(1, 10);

    connect(this, SIGNAL(signalUpdateByTimer()),
            parea, SLOT(slotUpdateByTimer()));

    connect(pctrl, SIGNAL(signalAddressChanged(AttributeType *)),
            parea, SLOT(slotAddressChanged(AttributeType *)));
}


void MemViewWidget::slotUpdateByTimer() {
    if (!isVisible()) {
        return;
    }
    emit signalUpdateByTimer();
}

}  // namespace debugger
