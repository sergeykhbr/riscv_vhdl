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

#include "MainWindow/UnclosableWidget.h"
#include <QtWidgets/QWidget>
#include <QtCore/QTimer>
#include <QtGui/QResizeEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QCloseEvent>

namespace debugger {

class GpioWidget : public UnclosableWidget,
                   public IGuiCmdHandler {
    Q_OBJECT
public:
    GpioWidget(IGui *igui, QWidget *parent = 0);

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalClose(QWidget *, AttributeType &);
    void signalLedValue(uint32_t value);
    void signalDipValue(uint32_t value);

private slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByTimer();

protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE;

private:
    IGui *igui_;

    AttributeType cmdRd_;
    GpioType value_;
    GpioType newValue_;
};

}  // namespace debugger
