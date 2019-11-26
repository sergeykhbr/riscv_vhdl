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
#include "coreservices/irawlistener.h"
#include "coreservices/iserial.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtGui/qevent.h>

namespace debugger {

/**
 * QPlainTextEdit gives per-line scrolling (but optimized for plain text)
 * QTextEdit gives smooth scrolling (line partial move up-down)
 */
class UartEditor : public QPlainTextEdit,
                   public IRawListener {
    Q_OBJECT
public:
    UartEditor(IGui *igui, QWidget *parent = 0);
    ~UartEditor();

    // IRawListener
    virtual int updateData(const char *buf, int buflen);

signals:
    void signalClose(QWidget *, AttributeType &);
    void signalNewData();

private slots:
    void slotUpdateByData();
    
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void closeEvent(QCloseEvent *event_);

private:
    char keyevent2char(QKeyEvent *e);

private:
    IGui *igui_;
    ISerial *uart_;
    QString strOutput_;
    mutex_def mutexStr_;
    char prevSymb_;
};

}  // namespace debugger
