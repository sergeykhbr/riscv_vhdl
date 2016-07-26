/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      LED's emulator.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"

#include <QtWidgets/QWidget>

namespace debugger {

class RegWidget : public QWidget {
    Q_OBJECT
public:
    explicit RegWidget(const char *name, QWidget *parent);

signals:
    void signalChanged(const char *name, uint64_t val);
private slots:
    void slotPollingUpdate(const char *name, uint64_t val);

private:
    const char *name_;
    uint64_t value_;
};

}  // namespace debugger
