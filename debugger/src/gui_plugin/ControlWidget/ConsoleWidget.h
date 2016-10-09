/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Serial console emulator.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "coreservices/iautocomplete.h"
#include "coreservices/iconsole.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtGui/qevent.h>

namespace debugger {

/**
 * QPlainTextEdit gives per-line scrolling (but optimized for plain text)
 * QTextEdit gives smooth scrolling (line partial move up-down)
 */
class ConsoleWidget : public QPlainTextEdit,
                      public IConsole {
    Q_OBJECT
public:
    ConsoleWidget(IGui *igui, QWidget *parent = 0);
    ~ConsoleWidget();

    // IConsole
    virtual void writeBuffer(const char *buf);
    virtual void writeCommand(const char *cmd) {}
    virtual int registerKeyListener(IFace *iface) { return 0; }
    virtual void setCmdString(const char *buf) {}
    virtual void enableLogFile(const char *filename) {}

signals:
    void signalClose(QWidget *, AttributeType &);
private slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByTimer();
    void slotClosingMainForm();

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void closeEvent(QCloseEvent *event_);

private:
    char keyevent2char(QKeyEvent *e);
    char *qstring2cstr(QString s);
    QString getCommandLine();

private:
    AttributeType consoleListeners_;

    IGui *igui_;

    int cursorMinPos_;
    wchar_t *wcsConv_;
    char *mbsConv_;
    int sizeConv_;
    mutex_def mutexOutput_;
    QString strOutput_;
    QFont fontMainText_;
    QFont fontRISCV_;
};

}  // namespace debugger
