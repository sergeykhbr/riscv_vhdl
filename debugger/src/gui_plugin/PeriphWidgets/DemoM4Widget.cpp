/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "LedDisplay.h"
#include "DemoSTM32Keypad.h"
#include "DemoM4Widget.h"
#include <memory>

namespace debugger {

DemoM4Widget::DemoM4Widget(IGui *igui, QWidget *parent) : QWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);

    LedDisplay *parea = new LedDisplay(igui, this, "display0");
    connect(this, SIGNAL(signalUpdateByTimer()),
            parea, SLOT(slotUpdateByTimer()));

    DemoSTM32Keypad *pkpad = new DemoSTM32Keypad(this, igui);
    connect(this, SIGNAL(signalUpdateByTimer()),
            pkpad, SLOT(slotUpdateByTimer()));

    gridLayout->addWidget(parea, 0, 0, Qt::AlignCenter);
    gridLayout->addWidget(pkpad, 1, 0, Qt::AlignCenter);

    gridLayout->setRowStretch(2, 10);
    gridLayout->setColumnStretch(1, 10);
    setLayout(gridLayout);

    //connect(this, SIGNAL(signalUpdateByTimer()),
    //        pbtn, SLOT(slotUpdateByTimer()));
}

void DemoM4Widget::slotUpdateByTimer() {
    if (!isVisible()) {
        return;
    }
    emit signalUpdateByTimer();
}

}  // namespace debugger
