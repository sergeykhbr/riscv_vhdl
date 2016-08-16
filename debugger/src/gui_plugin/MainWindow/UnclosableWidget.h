/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Widget wrapper.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtGui/qevent.h>

namespace debugger {

class UnclosableWidget : public QWidget {
    Q_OBJECT
public:
    UnclosableWidget(QWidget *parent = 0) : QWidget(parent) {}

signals:
    void signalResize(QSize sz);
    
protected:
};

}  // namespace debugger
