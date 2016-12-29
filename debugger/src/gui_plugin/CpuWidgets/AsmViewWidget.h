/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer form.
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

class AsmViewWidget : public UnclosableWidget {
    Q_OBJECT
public:
    AsmViewWidget(IGui *igui, QWidget *parent = 0);

signals:
    void signalPostInit(AttributeType *cfg);
    void signalUpdateByTimer();

private slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByTimer();

private:
    AttributeType listMem_;
    QGridLayout *gridLayout;
    
    IGui *igui_;
};

}  // namespace debugger
