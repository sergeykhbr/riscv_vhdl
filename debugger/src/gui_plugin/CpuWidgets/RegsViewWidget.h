/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Single CPU register form.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include "MainWindow/UnclosableWidget.h"
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtCore/QEvent>

namespace debugger {

class RegsViewWidget : public UnclosableWidget,
                       public IGuiCmdHandler {
    Q_OBJECT
public:
    RegsViewWidget(IGui *igui, QWidget *parent = 0);
    virtual ~RegsViewWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalHandleResponse(AttributeType *resp);

private slots:
    void slotUpdateByTimer();

private:
    void addRegWidget(int idx, const char *name);

private:
    AttributeType cmdRegs_;
    AttributeType listRegs_;
    AttributeType resp_;
    QGridLayout *gridLayout;
    
    IGui *igui_;
    bool waitingResp_;
};

}  // namespace debugger
