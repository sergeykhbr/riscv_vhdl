/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Code Coverage widgets.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtCore/QEvent>
#include <QtGui/qevent.h>

namespace debugger {

class CodeCoverageWidget : public QWidget,
                           public IGuiCmdHandler {
    Q_OBJECT
 public:
    CodeCoverageWidget(IGui *igui, QWidget *parent);
    virtual ~CodeCoverageWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

    /** Accessors methods */
    AttributeType *getpDetailedInfo() { return &respDetailedInfo_; }

 signals:
    void signalDetailedInfoUpdate();

 private slots:
    void slotUpdateByTimer();
    void slotDetailedInfoRequest();

 private:
    AttributeType cmdBriefInfo_;
    AttributeType respBriefInfo_;
    AttributeType cmdDetailedInfo_;
    AttributeType respDetailedInfo_;
    QGridLayout *gridLayout;
    QLabel *lblCoverage_;

    IGui *igui_;
    volatile bool requested_;
    volatile bool requested_detailed_;
    double coverage_;
};

class CodeCoverageQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
 public:
    CodeCoverageQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                    QAction *act = 0) : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("Code Coverage"));
        QWidget *pnew = new CodeCoverageWidget(igui, this);
        if (act) {
            setWindowIcon(act->icon());
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
