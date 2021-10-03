/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer form.
 */

#include "AsmArea.h"
#include "AsmControl.h"
#include "AsmViewWidget.h"
#include <memory>

namespace debugger {

AsmViewWidget::AsmViewWidget(IGui *igui, QWidget *parent, uint64_t fixaddr) 
    : QWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);

    AsmControl *pctrl = new AsmControl(this);

    AsmArea *parea = new AsmArea(igui, this, fixaddr);
    gridLayout->addWidget(pctrl , 0, 0);
    gridLayout->addWidget(parea, 1, 0);
    gridLayout->setRowStretch(1, 10);

    connect(this, SIGNAL(signalUpdateByTimer()),
            parea, SLOT(slotUpdateByTimer()));

    connect(parea, SIGNAL(signalBreakpointsChanged()),
            this, SLOT(slotBreakpointsChanged()));

    connect(this, SIGNAL(signalRedrawDisasm()),
            parea, SLOT(slotRedrawDisasm()));
}

}  // namespace debugger
