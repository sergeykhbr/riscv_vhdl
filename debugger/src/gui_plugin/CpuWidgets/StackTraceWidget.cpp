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
 * @brief      Stack trace viewer form.
 */

#include "StackTraceArea.h"
#include "StackTraceWidget.h"
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
