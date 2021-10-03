/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "RegsControl.h"
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QComboBox>

namespace debugger {

RegsControl::RegsControl(IGui *igui, QWidget *parent) 
    : QWidget(parent), igui_(igui) {
    QFont font = QFont("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    QFontMetrics fm(font);

    QGridLayout *gridLayout = new QGridLayout(this);
    setLayout(gridLayout);


    QComboBox *box = new QComboBox(this);
    
    const AttributeType &cfg = (*igui_->getpConfig())["RegsViewWidget"];
    if (!cfg.is_dict()) {
        return;
    }

    char tstr[256];
    for (unsigned i = 0; i < cfg["CpuContext"].size(); i++) {
        const AttributeType &ctx = cfg["CpuContext"][i];
        RISCV_sprintf(tstr, sizeof(tstr), "cpu[%d]: %s",
            ctx["CpuIndex"].to_int(), ctx["Description"].to_string());
        box->addItem(tstr);
    }
    box->setCurrentIndex(0);

    gridLayout->addWidget(box, 0, 0, Qt::AlignLeft);
    gridLayout->setColumnStretch(1, 10);

    connect(box, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotCurrentIndexChanged(int)));

    //cmd_.make_list(2);
}

void RegsControl::slotCurrentIndexChanged(int idx) {
    emit signalContextSwitched(idx);
}

}  // namespace debugger
