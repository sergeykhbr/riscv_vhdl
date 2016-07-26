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
#include <QtCore/qtimer.h>
#include <QtGui/qevent.h>

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
private slots:
    void slotConfigure(AttributeType *cfg);
    void slotRepaintByTimer();
    void slotClosingMainForm();
    void slotPollingUpdate();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *event_);    // call on update() or redraw()
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE;

private:
    QTimer *pollingTimer_;

    IGui *igui_;
    ISignal *igpio_;
    bool needUpdate_;
    uint64_t dips_;
    uint64_t leds_;
    QPixmap pixmapBkg_;
    QPixmap pixmapGreen_;
    QPixmap pixmapGrey_;
};

}  // namespace debugger
