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
    explicit RegWidget(AttributeType *cfg, QWidget *parent);

signals:
    void signalChanged(QString &name, uint64_t val);
private slots:
    void slotUpdate(QString &name, uint64_t val);

private:
    QString name_;
    uint64_t addr_;
    uint64_t value_;
};

}  // namespace debugger
