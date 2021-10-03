/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "RegSetView.h"
#include "RegsControl.h"
#include "RegsViewWidget.h"
#include <QtWidgets/QLabel>

#include <memory>

namespace debugger {

RegsAreaWidget::RegsAreaWidget(IGui *igui, QWidget *parent) : QWidget(parent) {
    QGridLayout *gridLayout = new QGridLayout(this);
    setLayout(gridLayout);

    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *pctrl = new RegsControl(igui, this);
    gridLayout->addWidget(pctrl, 0, 0);

    QWidget *pregs = new RegSetView(igui, this, 0);
    gridLayout->addWidget(pregs, 1, 0);

    connect(this, SIGNAL(signalUpdateByTimer()),
            pregs, SLOT(slotUpdateByTimer()));

    connect(pctrl, SIGNAL(signalContextSwitched(int)),
            pregs, SLOT(slotContextSwitchRequest(int)));
}

RegsAreaWidget::~RegsAreaWidget() {
}

}  // namespace debugger
