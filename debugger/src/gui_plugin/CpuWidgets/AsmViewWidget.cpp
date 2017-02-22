/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer form.
 */

#include "SymbolBrowserArea.h"
#include "SymbolBrowserControl.h"
#include "SymbolBrowserWidget.h"
#include "moc_SymbolBrowserWidget.h"

#include <memory>

namespace debugger {

SymbolBrowserWidget::SymbolBrowserWidget(IGui *igui, QWidget *parent)
    : QWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);

    SymbolBrowserControl *pctrl = new SymbolBrowserControl(this);

    SymbolBrowserArea *parea = new SymbolBrowserArea(igui, this);
    gridLayout->addWidget(pctrl , 0, 0);
    gridLayout->addWidget(parea, 1, 0);
    gridLayout->setRowStretch(1, 10);
}

}  // namespace debugger
