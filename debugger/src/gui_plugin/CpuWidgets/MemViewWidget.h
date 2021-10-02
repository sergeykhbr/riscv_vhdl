/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory editor form.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtCore/QEvent>

namespace debugger {

class MemViewWidget : public QWidget {
    Q_OBJECT
public:
    MemViewWidget(IGui *igui, QWidget *parent, uint64_t addr, uint64_t sz);

signals:
    void signalUpdateByTimer();

private slots:
    void slotUpdateByTimer();

private:
    AttributeType listMem_;
    QGridLayout *gridLayout;
    
    IGui *igui_;
};

class MemQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    MemQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                    uint64_t addr, uint64_t sz, QAction *act = 0)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("Memory"));
        QWidget *pnew = new MemViewWidget(igui, this, addr, sz);
        setWindowIcon(QIcon(tr(":/images/mem_96x96.png")));
        if (act) {
            act->setChecked(true);
            connect(parent, SIGNAL(signalUpdateByTimer()),
                    pnew, SLOT(slotUpdateByTimer()));
        }
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
