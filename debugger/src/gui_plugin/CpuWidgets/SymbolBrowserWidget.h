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
#include <QtGui/qevent.h>

namespace debugger {

class SymbolBrowserWidget : public QWidget {
    Q_OBJECT
public:
    SymbolBrowserWidget(IGui *igui, QWidget *parent);

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
        setWidget(new SymbolBrowserWidget(igui, this));
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
