/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      MDI SubWindow wrapper widget.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/qmdisubwindow.h>
#include <QtGui/qevent.h>

namespace debugger {

class UnclosableQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    UnclosableQMdiSubWindow(QWidget *parent = 0) : QMdiSubWindow(parent) {}

public:
    void setUnclosableWidget(QWidget *widget) {
        setWidget(widget);
        connect(widget, SIGNAL(signalResize(QSize)), 
                this, SLOT(slotResize(QSize)));
    }

signals:
    void signalVisible(bool);

private slots:
    void slotVisible(bool val) {
        if (val) {
            show();
        } else {
            hide();
        }
    }
    void slotResize(QSize sz) {
        resize(sz);
    }
    
protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE {
        setVisible(false);
        emit signalVisible(false);
        event_->ignore();
    }
};

#include "moc_UnclosableQMdiSubWindow.h"

}  // namespace debugger
