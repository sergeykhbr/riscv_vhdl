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
#include "coreservices/isocinfo.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>

namespace debugger {

class RegWidget : public QWidget,
                  public IGuiCmdHandler {
    Q_OBJECT
public:
    explicit RegWidget(const char *name, IGui *gui, QWidget *parent);

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    //void signalChanged(uint64_t idx, uint64_t val);
private slots:
    void slotUpdateByTimer();

private:
    AttributeType cmdRead_;
    QString name_;
    IGui *igui_;
    QLineEdit *edit_;
    uint64_t value_;
};

}  // namespace debugger
