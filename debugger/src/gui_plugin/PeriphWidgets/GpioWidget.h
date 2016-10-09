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
#include "coreservices/isignal.h"
#include "coreservices/isignallistener.h"

#include <QtWidgets/QWidget>
#include <QtCore/QTimer>
#include <QtGui/QResizeEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QCloseEvent>

namespace debugger {

class GpioWidget : public QWidget,
                  public ISignalListener,
                  public IGuiCmdHandler {
    Q_OBJECT
public:
    GpioWidget(IGui *igui, QWidget *parent = 0);
    ~GpioWidget();

    // ISignalListener
    virtual void updateSignal(int start, int width, uint64_t value);

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalClose(QWidget *, AttributeType &);
    void signalLedValue(uint32_t value);
    void signalDipValue(uint32_t value);

private slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByTimer();
    void slotClosingMainForm();

protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE;

private:

    IGui *igui_;
    ISignal *igpio_;

    AttributeType cmdRd_;
};

}  // namespace debugger
