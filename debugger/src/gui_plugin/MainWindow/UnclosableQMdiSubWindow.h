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
    UnclosableQMdiSubWindow(QWidget *parent = 0, 
                            bool add_scroll = false,
                            bool donot_close = true)
        : QMdiSubWindow(parent) {
        donot_close_ = donot_close;
        if (add_scroll) {
            scrollArea_ = new QScrollArea(parent);
            setWidget(scrollArea_);
        } else {
            scrollArea_ = 0;
        }
        if (donot_close == false) {
            setAttribute(Qt::WA_DeleteOnClose);
        }
    }

public:
    void setUnclosableWidget(const char *title, QWidget *widget, QAction *act) {
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
        setWindowTitle(tr(title));
        setWindowIcon(act->icon());
        connect(act, SIGNAL(triggered(bool)), this, SLOT(slotVisible(bool)));
        connect(this, SIGNAL(signalVisible(bool)), act, SLOT(setChecked(bool)));
        setVisible(act->isChecked());
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
    
protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE {
        setVisible(false);
        emit signalVisible(false);
        if (donot_close_) {
            event_->ignore();
        } else {
            event_->accept();
        }
    }
private:
    QScrollArea *scrollArea_;
    bool donot_close_;
};

#include "moc_UnclosableQMdiSubWindow.h"

}  // namespace debugger
