/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
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
 *
 * @brief      Disassembler viewer form.
 */

#include "SymbolBrowserArea.h"
#include "SymbolBrowserControl.h"
#include "SymbolBrowserWidget.h"
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

    connect(pctrl, SIGNAL(signalFilterChanged(const QString &)),
            parea, SLOT(slotFilterChanged(const QString &)));

    connect(parea, SIGNAL(signalShowFunction(uint64_t, uint64_t)),
            this, SLOT(slotShowFunction(uint64_t, uint64_t)));

    connect(parea, SIGNAL(signalShowData(uint64_t, uint64_t)),
            this, SLOT(slotShowData(uint64_t, uint64_t)));
}

}  // namespace debugger
