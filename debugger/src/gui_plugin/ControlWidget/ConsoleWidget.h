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
#include "coreservices/irawlistener.h"
#include "coreservices/icmdexec.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtGui/qevent.h>

namespace debugger {

/**
 * QPlainTextEdit gives per-line scrolling (but optimized for plain text)
 * QTextEdit gives smooth scrolling (line partial move up-down)
 */
class ConsoleWidget : public QPlainTextEdit,
                      public IGuiCmdHandler,
                      public IRawListener {
    Q_OBJECT
public:
    ConsoleWidget(IGui *igui, QWidget *parent = 0);
    ~ConsoleWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

    // IRawListener
    virtual void updateData(const char *buf, int buflen);

signals:
    void signalNewData();
private slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByData();

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private:
    int keyevent2sequence(QKeyEvent *e, uint8_t *seq);
    char *qstring2cstr(QString s);

private:
    IGui *igui_;
    IAutoComplete *iauto_;

    AttributeType cursorPos_;

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
