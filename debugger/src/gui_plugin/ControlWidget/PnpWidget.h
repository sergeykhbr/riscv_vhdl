/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plig'n'play information representation window.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include "MainWindow/UnclosableWidget.h"
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>

namespace debugger {

class PnpWidget : public UnclosableWidget,
                  public IGuiCmdHandler {
    Q_OBJECT
public:
    PnpWidget(IGui *igui, QWidget *parent = 0);
    ~PnpWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:

private slots:
    void slotConfigure(AttributeType *cfg);

protected:
    void paintEvent(QPaintEvent *event_);    // call on update() or redraw()

private:
    IGui *igui_;
    QGridLayout *mainLayout_;
    QLabel *labelBoard_;
    QPixmap pixmapBkg_;
};

}  // namespace debugger
