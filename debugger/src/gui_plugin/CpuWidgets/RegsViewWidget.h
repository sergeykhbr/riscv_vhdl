/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      LED's emulator.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/qmdisubwindow.h>
#include <QtCore/qtimer.h>
#include <QtGui/qevent.h>

namespace debugger {

class RegsViewWidget : public QWidget,
                       public IGuiCmdHandler {
    Q_OBJECT
public:
    RegsViewWidget(IGui *igui, QWidget *parent = 0);

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

//signals:
//    void signalClose(QWidget *, AttributeType &);

private slots:
    void slotConfigure(AttributeType *cfg);
    void slotPollingUpdate();

protected:

private:
    QTimer *pollingTimer_;

    IGui *igui_;
};

}  // namespace debugger
