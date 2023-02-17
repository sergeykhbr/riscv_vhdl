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
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>

namespace debugger {

RegWidget::RegWidget(const char *name, int bytes, QWidget *parent) 
    : QLineEdit(parent) {
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

    RISCV_sprintf(fmtValue_, sizeof(fmtValue_), "%%0%d" RV_PRI64 "x", 2*bytes);
    respValue_ = value_ = 0xfeedfaceull;

    char tstr[64];
    RISCV_sprintf(tstr, sizeof(tstr), fmtValue_, value_);
    QString text(tstr);
    setText(text);
    setMaxLength(19);
    setFixedWidth(fm.horizontalAdvance(text) + 8);
    setFixedHeight(fm.height() + 2);

    setMinimumWidth(width() + 16);
    setMinimumHeight(height());

    connect(this, SIGNAL(editingFinished()),
            this, SLOT(slotEditingFinished()));
}

void RegWidget::slotHandleResponse(AttributeType *resp) {
    if (!resp->is_dict() || !resp->has_key(regName_.to_string())) {
        return;
    }
    outputValue((*resp)[regName_.to_string()]);
}

void RegWidget::outputValue(AttributeType &val) {
    respValue_ = val.to_uint64();
    if (respValue_ == value_) {
        return;
    }
    char tstr[64];
    value_ = respValue_;
    RISCV_sprintf(tstr, sizeof(tstr), fmtValue_, value_);
    QString text(tr(tstr));
    setText(text);
}

void RegWidget::slotEditingFinished() {
    wchar_t wcsConv[128];
    char mbsConv[128];
    int sz = text().toWCharArray(wcsConv);
    uint64_t new_val;
    wcstombs(mbsConv, wcsConv, sz);
    mbsConv[sz] = '\0';

    new_val = strtoull(mbsConv, 0, 16);

    if (new_val != value_) {
        char tstr[128];
        RISCV_sprintf(tstr, sizeof(tstr), "reg %s 0x%s",
                      regName_.to_string(), mbsConv);
        cmdWrite_.make_string(tstr);

        emit signalChanged(cmdWrite_.to_string());
        setFocus();
    }
}

}  // namespace debugger
