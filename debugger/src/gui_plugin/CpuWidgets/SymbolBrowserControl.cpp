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
 * @brief      Symbol Browser control panel.
 */

#include "SymbolBrowserControl.h"
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace debugger {

SymbolBrowserControl::SymbolBrowserControl(QWidget *parent) 
    : QWidget(parent) {
    QFont font = QFont("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    QFontMetrics fm(font);

    paletteDefault_.setColor(QPalette::Text, Qt::black);
    paletteDefault_.setColor(QPalette::Base, Qt::white);


    QGridLayout *gridLayout = new QGridLayout(this);
    setLayout(gridLayout);


    QLabel *lbl = new QLabel("Filter:");
    gridLayout->addWidget(lbl, 0, 0, Qt::AlignRight);

    editFilter_ = new QLineEdit(this);
    editFilter_->setText(tr("*"));
    editFilter_->setFixedWidth(fm.horizontalAdvance(tr("*some_test_func*")));
    editFilter_->setPalette(paletteDefault_);
    gridLayout->addWidget(editFilter_, 0, 1, Qt::AlignLeft);
    gridLayout->setColumnStretch(1, 10);

    connect(editFilter_, SIGNAL(returnPressed()),
            this, SLOT(slotFilterEditingFinished()));
}

void SymbolBrowserControl::slotFilterEditingFinished() {
    emit signalFilterChanged(editFilter_->text());
}



}  // namespace debugger
