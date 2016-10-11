#include "ConsoleWidget.h"
#include "moc_ConsoleWidget.h"

#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <memory>

namespace debugger {

static const char CONSOLE_ENTRY[] = "riscv# ";

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
    cursor.insertText(tr(CONSOLE_ENTRY));
    cursorMinPos_ = cursor.selectionStart();
    
    charFormat.setFont(fontMainText_);
    cursor.setCharFormat(charFormat);
    setTextCursor(cursor);
    setWindowTitle(tr("simconsole"));
}

ConsoleWidget::~ConsoleWidget() {
    RISCV_mutex_destroy(&mutexOutput_);
    delete [] wcsConv_;
    delete [] mbsConv_;
}

void ConsoleWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    RISCV_mutex_lock(&mutexOutput_);
    strOutput_ += QString(resp->to_config()) + "\n";
    RISCV_mutex_unlock(&mutexOutput_);

}

void ConsoleWidget::keyPressEvent(QKeyEvent *e) {
    char value = keyevent2char(e);


    int start;
    QTextCursor end_cursor;
    QTextCursor cursor = textCursor();
    start = cursor.selectionStart();
    /** Cannot edit previously printed lines */
    if (start < cursorMinPos_) {
        moveCursor(QTextCursor::End);
        cursor = textCursor();
        start = cursor.selectionStart();
    }

    AttributeType cmd, t2;
    if (iauto_->processKey(static_cast<uint8_t>(value), &cmd, &t2)) {
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(tr("\r"));

        QTextCharFormat charFormat = cursor.charFormat();
        cursor.insertText(tr(CONSOLE_ENTRY));
        cursorMinPos_ = cursor.selectionStart();
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());

        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmd, false);

    } else {
        cursor.insertText(e->text());
    }

#if 0
    switch (e->key()) {
    case Qt::Key_Left:
        if (start > cursorMinPos_) {
            moveCursor(QTextCursor::Left);
        }
        return;
    case Qt::Key_Right:
        moveCursor(QTextCursor::Right);
        return;
    case Qt::Key_Up:
        return;
    case Qt::Key_Down:
        return;
    case Qt::Key_Tab:
        return;
    case Qt::Key_Backspace:
        if (start > cursorMinPos_) {
            cursor.setPosition(start - 1, QTextCursor::KeepAnchor);
            cursor.insertText(tr(""));
        }
        return;
    case Qt::Key_Delete:
        moveCursor(QTextCursor::End);
        end_cursor = textCursor();
        setTextCursor(cursor);
        if (cursor.selectionStart() < end_cursor.selectionStart()) {
            cursor.setPosition(start + 1, QTextCursor::KeepAnchor);
            cursor.insertText(tr(""));
        }
        return;
    default:;
    }

    if (value == '\r' || value == '\n') {
        const char *cmd = qstring2cstr(getCommandLine()); 
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(tr("\r"));

        QTextCharFormat charFormat = cursor.charFormat();
        cursor.insertText(tr(CONSOLE_ENTRY));
        cursorMinPos_ = cursor.selectionStart();
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());

        AttributeType regcmd(cmd);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &regcmd, false);
    } else {
        cursor.insertText(e->text());
    }
#endif
}

void ConsoleWidget::closeEvent(QCloseEvent *event_) {
    AttributeType tmp;
    emit signalClose(this, tmp);
    event_->accept();
}

void ConsoleWidget::slotPostInit(AttributeType *cfg) {
    const char *autoobj = (*cfg)["AutoComplete"].to_string();
    iauto_ = static_cast<IAutoComplete *>(
        RISCV_get_service_iface(autoobj, IFACE_AUTO_COMPLETE));

    RISCV_add_default_output(static_cast<IRawListener *>(this));
}

void ConsoleWidget::slotUpdateByTimer() {
    if (strOutput_.size() == 0) {
        return;
    }

    QTextCursor cursor = textCursor();
    int delta = cursor.selectionStart();
    cursor.movePosition(QTextCursor::End);
    delta = cursor.selectionStart() - delta;

    cursor.movePosition(QTextCursor::StartOfLine);

    RISCV_mutex_lock(&mutexOutput_);
    cursor.insertText(strOutput_);

    cursorMinPos_ += strOutput_.size();
    strOutput_.clear();
    RISCV_mutex_unlock(&mutexOutput_);

    cursor.movePosition(QTextCursor::End);
    int end = cursor.selectionStart();
    cursor.setPosition(end - delta, QTextCursor::MoveAnchor);

    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ConsoleWidget::slotClosingMainForm() {
}

void ConsoleWidget::updateData(const char *buf, int bufsz) {
    RISCV_mutex_lock(&mutexOutput_);
    strOutput_ += QString(buf);
    RISCV_mutex_unlock(&mutexOutput_);
}

char ConsoleWidget::keyevent2char(QKeyEvent *e) {
    return qstring2cstr(e->text())[0];
}

char *ConsoleWidget::qstring2cstr(QString s) {
    int sz = s.size();
    if (sz >= sizeConv_) {
        sizeConv_ = sz + 1;
        delete [] wcsConv_;
        delete [] mbsConv_;
        wcsConv_ = new wchar_t[sizeConv_];
        mbsConv_ = new char[sizeConv_];
    }
    sz = s.toWCharArray(wcsConv_);
    wcstombs(mbsConv_, wcsConv_, sz);
    mbsConv_[sz] = '\0';
    return mbsConv_;
}

QString ConsoleWidget::getCommandLine() {
    QTextCursor cursor = textCursor();
    int prev = cursor.selectionStart();

    cursor.movePosition(QTextCursor::End);
    int start = cursor.selectionStart();
    int end = cursorMinPos_;
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    QString ret = cursor.selectedText();
    cursor.setPosition(prev, QTextCursor::MoveAnchor);
    return ret;
}

}  // namespace debugger
