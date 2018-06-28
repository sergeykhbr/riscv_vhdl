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

#include "RegWidget.h"
#include "moc_RegWidget.h"

#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>

namespace debugger {

RegWidget::RegWidget(const char *name, int bytes, QWidget *parent) 
    : QWidget(parent) {
    value_ = 0;

    regName_.make_string(name);
    name_ = QString(name);
    while (name_.size() < 3) {
        name_ += tr(" ");
    }

    std::string t1 = "reg " + std::string(name);
    cmdRead_.make_string(t1.c_str());

    QFont font = QFont("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    QFontMetrics fm(font);

    QHBoxLayout *pLayout = new QHBoxLayout;
    pLayout->setContentsMargins(4, 1, 4, 1);
    setLayout(pLayout);

    QLabel *label = new QLabel(this);
    QSizePolicy labelSizePolicy(QSizePolicy::Preferred, 
                                QSizePolicy::Preferred);
    labelSizePolicy.setHorizontalStretch(0);
    labelSizePolicy.setVerticalStretch(0);
    labelSizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(labelSizePolicy);
    label->setText(name_);
    label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    pLayout->addWidget(label);

    edit_ = new QLineEdit(this);
    pLayout->addWidget(edit_);
    RISCV_sprintf(fmtValue_, sizeof(fmtValue_), "%%0%d" RV_PRI64 "x", 2*bytes);
    respValue_ = value_ = 0xfeedfaceull;

    char tstr[64];
    RISCV_sprintf(tstr, sizeof(tstr), fmtValue_, value_);

    QString text(tstr);
    edit_->setText(text);
    edit_->setMaxLength(19);
    edit_->setFixedWidth(fm.width(text) + 8);
    edit_->setFixedHeight(fm.height() + 2);

    setMinimumWidth(edit_->width() + fm.width(name_) + 16);
    setMinimumHeight(edit_->height());

    connect(edit_, SIGNAL(editingFinished()),
            this, SLOT(slotEditingFinished()));
}

void RegWidget::slotHandleResponse(AttributeType *resp) {
    if (!resp->is_dict()) {
        return;
    }
    if (!resp->has_key(regName_.to_string())) {
        return;
    }
    respValue_ = (*resp)[regName_.to_string()].to_uint64();
    if (value_ != respValue_) {
        char tstr[64];
        value_ = respValue_;
        RISCV_sprintf(tstr, sizeof(tstr), fmtValue_, value_);
        QString text(tr(tstr));
        edit_->setText(text);
    }
}

void RegWidget::slotEditingFinished() {
    wchar_t wcsConv[128];
    char mbsConv[128];
    int sz = edit_->text().toWCharArray(wcsConv);
    uint64_t new_val;
    wcstombs(mbsConv, wcsConv, sz);
    mbsConv[sz] = '\0';

    new_val = strtoull(mbsConv, 0, 16);

    if (new_val != value_) {
        char tstr[128];
        RISCV_sprintf(tstr, sizeof(tstr), "reg %s 0x%s",
                      regName_.to_string(), mbsConv);
        cmdWrite_.make_string(tstr);

        emit signalChanged(&cmdWrite_);
        setFocus();
    }
}

}  // namespace debugger
