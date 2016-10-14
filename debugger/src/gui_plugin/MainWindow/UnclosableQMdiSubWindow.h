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
    UnclosableQMdiSubWindow(QWidget *parent = 0, bool add_scroll = false) 
        : QMdiSubWindow(parent) {
        if (add_scroll) {
            scrollArea_ = new QScrollArea(parent);
            //scrollArea_->setBackgroundRole(QPalette::Dark);
            setWidget(scrollArea_);
        } else {
            scrollArea_ = 0;
        }
    }

public:
    void setUnclosableWidget(QWidget *widget) {
        if (scrollArea_) {
            /**
             * Initial size of the widget won't change when we resize window,
             * so we must specify widget size to open scroll bars.
             */
            scrollArea_->setWidget(widget);
        } else {
            /**
             * Automatic resize widget size to the size of opened window
             */
            setWidget(widget);
        }
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
private:
    QScrollArea *scrollArea_;
};

#include "moc_UnclosableQMdiSubWindow.h"

}  // namespace debugger
