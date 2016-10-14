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
    void signalUpdate();

private slots:
    void slotConfigDone();
    void slotUpdate();

protected:
    virtual void showEvent(QShowEvent *event_);
    QLabel *getLabel(int id) {
        return static_cast<QLabel *>(mainLayout_->itemAt(id)->widget());
    }

private:
    IGui *igui_;
    QGridLayout *mainLayout_;
    
    AttributeType lstSlaves_;
    enum ETargets {SIMULATION, ML605, KC705, TARGET_Unknown, TARGETS_total};
    QImage imgTarget_[TARGETS_total];

    PnpMapType pnp_;
};

}  // namespace debugger
