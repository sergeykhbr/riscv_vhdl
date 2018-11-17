/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Editor control panel.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>

namespace debugger {

class MemControl : public QWidget {
    Q_OBJECT
public:
    explicit MemControl(QWidget *parent, uint64_t addr, uint64_t sz);

signals:
    void signalAddressChanged(AttributeType *cmd);

public slots:
    void slotModified();
    void slotUpdate();

private:
    bool isChanged();

private:
    QLineEdit *editAddr_;
    QLineEdit *editBytes_;
    QPalette paletteDefault_;
    QPalette paletteModified_;
    AttributeType cmd_;
};

}  // namespace debugger
