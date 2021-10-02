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
#include <QtGui/QAction>
#include <QtGui/qevent.h>

namespace debugger {

class SymbolBrowserWidget : public QWidget {
    Q_OBJECT
public:
    SymbolBrowserWidget(IGui *igui, QWidget *parent);

public slots:
    void slotShowFunction(uint64_t addr, uint64_t sz) {
        emit signalShowFunction(addr, sz);
    }
    void slotShowData(uint64_t addr, uint64_t sz) {
        emit signalShowData(addr, sz);
    }

signals:
    void signalShowFunction(uint64_t addr, uint64_t sz);
    void signalShowData(uint64_t addr, uint64_t sz);

private:
    QGridLayout *gridLayout;
    IGui *igui_;
};


class SymbolBrowserQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    SymbolBrowserQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent = 0)
        : QMdiSubWindow(parent) {
        area_ = area;
        setWindowTitle(tr("Symbol Browser"));
        setWindowIcon(QIcon(tr(":/images/info_96x96.png")));

        SymbolBrowserWidget *pWidget = new SymbolBrowserWidget(igui, this);
        setWidget(pWidget);
        connect(pWidget, SIGNAL(signalShowFunction(uint64_t, uint64_t)),
                parent, SLOT(slotOpenDisasm(uint64_t, uint64_t)));
        connect(pWidget, SIGNAL(signalShowData(uint64_t, uint64_t)),
                parent, SLOT(slotOpenMemory(uint64_t, uint64_t)));

        area_->addSubWindow(this);
        setAttribute(Qt::WA_DeleteOnClose);
        show();
    }

    
protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE {
        area_->removeSubWindow(this);
        event_->accept();
    }
private:
    QMdiArea *area_;
};



}  // namespace debugger
