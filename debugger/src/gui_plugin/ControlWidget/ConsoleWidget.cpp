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

#include "ConsoleWidget.h"
#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <memory>

namespace debugger {

static const char CONSOLE_ENTRY[] = "cmd# ";

ConsoleWidget::ConsoleWidget(IGui *igui, QWidget *parent) 
    : QPlainTextEdit(parent) {
    igui_ = igui;

    RISCV_mutex_init(&mutexOutput_);
    sizeConv_ = 1024;
    wcsConv_ = new wchar_t[sizeConv_];
    mbsConv_ = new char[sizeConv_];

    clear();
    fontMainText_ = QFont("Courier");
    fontMainText_.setStyleHint(QFont::Monospace);
    fontMainText_.setPointSize(9);
    fontMainText_.setFixedPitch(true);
    setFont(fontMainText_);

    fontRISCV_ = fontMainText_;
    fontRISCV_.setBold(true);

    ensureCursorVisible();

    QTextCursor cursor = textCursor();
    QTextCharFormat charFormat = cursor.charFormat();
    charFormat.setFont(fontRISCV_);
    cursor.setCharFormat(charFormat);

    cursor.insertText(tr(
    "**********************************************************\n"
    "  System Emulator and Debugger\n"
    "  Copyright (c) 2018 Sergey Khabarov\n"
    "  Licensed under the Apache License, Version 2.0.\n"
    "  e-mail: sergeykhbr@gmail.com\n"
    "**********************************************************\n"));

    cursor.insertText(tr(CONSOLE_ENTRY));
    cursorMinPos_ = cursor.selectionStart();
    
    charFormat.setFont(fontMainText_);
    cursor.setCharFormat(charFormat);
    setTextCursor(cursor);
    setWindowTitle(tr("simconsole"));

    cursorPos_.make_list(2);
    cursorPos_[0u].make_int64(0);
    cursorPos_[1].make_int64(0);

    connect(this, SIGNAL(signalNewData()), this, SLOT(slotUpdateByData()));

    const char *autoobj = (*igui_->getpConfig())["AutoComplete"].to_string();
    iauto_ = static_cast<IAutoComplete *>(
        RISCV_get_service_iface(autoobj, IFACE_AUTO_COMPLETE));

    RISCV_add_default_output(static_cast<IRawListener *>(this));
}

ConsoleWidget::~ConsoleWidget() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
    RISCV_remove_default_output(static_cast<IRawListener *>(this));
    RISCV_mutex_destroy(&mutexOutput_);
    delete [] wcsConv_;
    delete [] mbsConv_;
}

void ConsoleWidget::handleResponse(const char *cmd) {
    if (respcmd_.is_nil() || respcmd_.is_invalid()) {
        return;
    }
    RISCV_mutex_lock(&mutexOutput_);
    strOutput_ += QString(respcmd_.to_config().to_string()) + "\n";
    RISCV_mutex_unlock(&mutexOutput_);
    emit signalNewData();
}

void ConsoleWidget::keyPressEvent(QKeyEvent *e) {
    QTextCursor cursor = textCursor();
    uint32_t vt_key = static_cast<uint32_t>(e->nativeVirtualKey());
    char vt_char = static_cast<char>(vt_key);
    if (vt_char >= 'A' && vt_char <= 'Z' && e->modifiers() == Qt::NoModifier) {
        vt_key -= static_cast<uint32_t>('A');
        vt_key += static_cast<uint32_t>('a');
    }

    if (e->text().size()) {
        uint8_t spsmb = static_cast<uint8_t>(e->text().at(0).toLatin1());
        if (spsmb == '_' || spsmb == ':' || spsmb == '\\' || spsmb == '/'
         || spsmb == '\'' || spsmb == '[' || spsmb == ']'
         || spsmb == '{' || spsmb == '}' || spsmb == ',' || spsmb == '.'
         || spsmb == '-' || spsmb == '+') {
            vt_key = spsmb;
        }
    }
    //printf("vt_key = %08x\n", vt_key);
    bool cmd_ready = iauto_->processKey(vt_key, &reqcmd_, &cursorPos_);

    moveCursor(QTextCursor::End);
    cursor = textCursor();
    cursor.setPosition(cursorMinPos_, QTextCursor::KeepAnchor);
    cursor.insertText(reqcmd_.to_string());
    if (cursorPos_[0u].to_int()) {
        cursor.movePosition(QTextCursor::Left, 
                QTextCursor::MoveAnchor, cursorPos_[0u].to_int());
        setTextCursor(cursor);
    }

    if (!cmd_ready) {
        return;
    }
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(tr("\r"));

    QTextCharFormat charFormat = cursor.charFormat();
    cursor.insertText(tr(CONSOLE_ENTRY));
    cursorMinPos_ = cursor.selectionStart();
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
        reqcmd_.to_string(), &respcmd_, false);
}

void ConsoleWidget::slotUpdateByData() {
    if (strOutput_.size() == 0) {
        return;
    }
    // Keep current line value:
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    QString cur_line = cursor.selectedText();
    // Insert raw string:
    RISCV_mutex_lock(&mutexOutput_);
    cursor.insertText(strOutput_);
    cursorMinPos_ += strOutput_.size();
    strOutput_.clear();
    RISCV_mutex_unlock(&mutexOutput_);

    // Restore line:
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(cur_line);
    // Restore cursor position:
    cursor.movePosition(QTextCursor::End);
    cursor.movePosition(QTextCursor::Left, 
           QTextCursor::MoveAnchor, cursorPos_[0u].to_int());
    setTextCursor(cursor);

    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

int ConsoleWidget::updateData(const char *buf, int bufsz) {
    RISCV_mutex_lock(&mutexOutput_);
    strOutput_ += QString(buf);
    RISCV_mutex_unlock(&mutexOutput_);
    emit signalNewData();
    return bufsz;
}

}  // namespace debugger
