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

#include "MainWindow/UnclosableWidget.h"
#include <QtWidgets/qmdisubwindow.h>
#include <QtWidgets/qgridlayout.h>
#include <QtCore/qtimer.h>
#include <QtGui/qevent.h>

namespace debugger {

class RegsViewWidget : public UnclosableWidget,
                       public IGuiCmdHandler {
    Q_OBJECT
public:
    RegsViewWidget(IGui *igui, QWidget *parent = 0);

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalRegisterValue(uint64_t reg_idx, uint64_t val);

private slots:
    void slotConfigure(AttributeType *cfg);
    void slotPollingUpdate();
    void slotTargetStateChanged(bool);

private:
    int widgetIndexByName(const char *regname);
    void addRegWidget(AttributeType &reg_cfg);

private:
    AttributeType listRegs_;
    QGridLayout *gridLayout;
    QTimer *pollingTimer_;

    IGui *igui_;
    bool minSizeApplied_;
};

}  // namespace debugger
