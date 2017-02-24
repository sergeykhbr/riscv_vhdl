/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Serial console emulator.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "coreservices/irawlistener.h"
#include "coreservices/iserial.h"

#include "UartEditor.h"
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QAction>
#include <QtCore/QEvent>

namespace debugger {

class UartWidget : public QWidget {
    Q_OBJECT
public:
    UartWidget(IGui *igui, QWidget *parent);
    
signals:
    void signalPostInit(AttributeType *cfg);

private slots:
    void slotPostInit(AttributeType *cfg);

private:
    UartEditor *editor_;
};

class UartQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    UartQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                      QAction *act = 0)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("uart0"));
        setMinimumWidth(parent->size().width() / 2);
        QWidget *pnew = new UartWidget(igui, this);
        setWindowIcon(QIcon(tr(":/images/serial_96x96.png")));
        if (act) {
            act->setChecked(true);
        }
        connect(parent, SIGNAL(signalPostInit(AttributeType *)),
                pnew, SLOT(slotPostInit(AttributeType *)));
        setWidget(pnew);
        area_->addSubWindow(this);
        show();
    }
    
protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE {
        if (action_) {
            action_->setChecked(false);
        }
        area_->removeSubWindow(this);
        event_->accept();
    }
private:
    QAction *action_;
    QMdiArea *area_;
};

}  // namespace debugger
