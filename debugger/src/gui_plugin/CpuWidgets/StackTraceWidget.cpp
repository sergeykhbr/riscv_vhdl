/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Stack trace viewer form.
 */

#include "StackTraceArea.h"
#include "StackTraceWidget.h"
#if!defined(CMAKE_ENABLED)
#include "moc_StackTraceWidget.h"
#endif
#include <memory>

namespace debugger {

StackTraceWidget::StackTraceWidget(IGui *igui, QWidget *parent)
    : QWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);

    StackTraceArea *parea = new StackTraceArea(igui, this);
    gridLayout->addWidget(parea, 0, 0);
    gridLayout->setRowStretch(0, 10);

    connect(this, SIGNAL(signalUpdateByTimer()),
            parea, SLOT(slotUpdateByTimer()));

    connect(parea, SIGNAL(signalShowFunction(uint64_t, uint64_t)),
            this, SLOT(slotShowFunction(uint64_t, uint64_t)));
}

}  // namespace debugger
