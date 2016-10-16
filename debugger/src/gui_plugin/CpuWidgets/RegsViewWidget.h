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
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtCore/QEvent>

namespace debugger {

class RegsViewWidget : public UnclosableWidget {
    Q_OBJECT
public:
    RegsViewWidget(IGui *igui, QWidget *parent = 0);

signals:
    void signalUpdateByTimer();

private slots:
    void slotUpdateByTimer();

private:
    void addRegWidget(int idx, const char *name);

private:
    AttributeType listRegs_;
    QGridLayout *gridLayout;
    
    IGui *igui_;
};

}  // namespace debugger
