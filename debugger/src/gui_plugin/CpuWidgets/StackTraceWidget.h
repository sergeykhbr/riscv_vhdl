/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Stack trace viewer form.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtGui/qevent.h>

namespace debugger {

class StackTraceWidget : public QWidget {
    Q_OBJECT
public:
    StackTraceWidget(IGui *igui, QWidget *parent);

signals:
    void signalUpdateByTimer();
    void signalShowFunction(uint64_t addr, uint64_t sz);

private slots:
    void slotUpdateByTimer() {
        emit signalUpdateByTimer();
    }
    void slotShowFunction(uint64_t addr, uint64_t sz) {
        emit signalShowFunction(addr, sz);
    }

private:
    QGridLayout *gridLayout;
    IGui *igui_;
};


class StackTraceQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    StackTraceQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                      QAction *act = 0)
        : QMdiSubWindow(parent) {
        area_ = area;
        action_ = act;
        setWindowTitle(tr("Stack Trace"));
        setWindowIcon(QIcon(tr(":/images/stack_96x96.png")));

        StackTraceWidget *pnew = new StackTraceWidget(igui, this);
        setWidget(pnew);
        if (act) {
            act->setChecked(true);
        }
        connect(parent, SIGNAL(signalUpdateByTimer()),
                pnew, SLOT(slotUpdateByTimer()));

        connect(pnew, SIGNAL(signalShowFunction(uint64_t, uint64_t)),
                parent, SLOT(slotOpenDisasm(uint64_t, uint64_t)));

        area_->addSubWindow(this);
        setAttribute(Qt::WA_DeleteOnClose);
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
