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
 */

#include "UartEditor.h"
#include <QtWidgets/QScrollBar>
#include <memory>

namespace debugger {

UartEditor::UartEditor(IGui *igui, QWidget *parent) : QPlainTextEdit(parent) {
    igui_ = igui;
    uart_ = 0;

    clear();
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);

    ensureCursorVisible();

    /** Set background color */
    QPalette p = palette();
    p.setColor(QPalette::Active, QPalette::Base, Qt::lightGray);
    p.setColor(QPalette::Inactive, QPalette::Base, Qt::lightGray);
    setPalette(p);

    /** */
    RISCV_mutex_init(&mutexStr_);
    prevSymb_ = 0;

    connect(this, SIGNAL(signalNewData()), this, SLOT(slotUpdateByData()));
    const AttributeType *cfg = igui_->getpConfig();
    const AttributeType &serial_name = (*cfg)["Serial"];
    if (serial_name.is_string()) {
        uart_ = static_cast<ISerial *>
            (RISCV_get_service_iface(serial_name.to_string(), IFACE_SERIAL));
        if (uart_) {
            uart_->registerRawListener(static_cast<IRawListener *>(this));
        }
    }
}

UartEditor::~UartEditor() {
    if (uart_) {
        uart_->unregisterRawListener(static_cast<IRawListener *>(this));
    }
    RISCV_mutex_destroy(&mutexStr_);
}

void UartEditor::keyPressEvent(QKeyEvent *e) {
    QString str1 = e->text();
    if (str1.size() == 0) {
        return;
    }
    char symb = keyevent2char(e);
    if (uart_) {
        uart_->writeData(&symb, 1);
    }
}

void UartEditor::closeEvent(QCloseEvent *event_) {
    AttributeType tmp;
    emit signalClose(this, tmp);
    event_->accept();
}

int UartEditor::updateData(const char *buf, int buflen) {
    RISCV_mutex_lock(&mutexStr_);
    while (buflen--) {
        // Zephyr kernel scan symbol '\n' and after it adds the extra
        // symbol '\r', which I'm removing here.
        if (prevSymb_ == '\n' && buf[0] == '\r') {
            buf++;
            continue;
        }
        strOutput_ += buf[0];
        prevSymb_ = buf[0];
        buf++;
    }
    RISCV_mutex_unlock(&mutexStr_);

    emit signalNewData();
    return buflen;
}

void UartEditor::slotUpdateByData() {
    if (!strOutput_.size()) {
        return;
    }

    RISCV_mutex_lock(&mutexStr_);
    moveCursor(QTextCursor::End);
    QTextCursor cursor = textCursor();
    cursor.insertText(strOutput_);
    strOutput_.clear();
    RISCV_mutex_unlock(&mutexStr_);

    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

char UartEditor::keyevent2char(QKeyEvent *e) {
    wchar_t w1[8];
    char s1[8];
    int w1_sz = e->text().toWCharArray(w1);
    wcstombs(s1, w1, w1_sz);
    return s1[0];
}

}  // namespace debugger
