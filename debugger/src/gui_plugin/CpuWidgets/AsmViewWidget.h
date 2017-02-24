/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer form.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QAction>
#include <QtCore/QEvent>

namespace debugger {

class AsmViewWidget : public QWidget {
    Q_OBJECT
public:
    AsmViewWidget(IGui *igui, QWidget *parent, uint64_t fixaddr);

signals:
    void signalPostInit(AttributeType *cfg);
    void signalUpdateByTimer();
    void signalBreakpoint();

private slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByTimer();
    void slotBreakpoint();

private:
    AttributeType listMem_;
    QGridLayout *gridLayout;
    
    IGui *igui_;
};


class AsmQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    AsmQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent = 0,
                    QAction *act = 0, uint64_t fixaddr = ~0ull)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;
        setWindowTitle(tr("Disassembler"));
        QWidget *pnew = new AsmViewWidget(igui, this, fixaddr);
        setWindowIcon(QIcon(tr(":/images/asm_96x96.png")));
        if (act) {
            act->setChecked(true);
            connect(parent, SIGNAL(signalUpdateByTimer()),
                    pnew, SLOT(slotUpdateByTimer()));
        }
        connect(parent, SIGNAL(signalPostInit(AttributeType *)),
                pnew, SLOT(slotPostInit(AttributeType *)));
        connect(parent, SIGNAL(signalBreakpoint()),
                pnew, SLOT(slotBreakpoint()));
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
