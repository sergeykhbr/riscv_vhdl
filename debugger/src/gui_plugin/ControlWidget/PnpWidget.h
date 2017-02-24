/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Plig'n'play information representation window.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QLabel>
#include <QtCore/QEvent>
#include <QtGui/qevent.h>

namespace debugger {

class PnpWidget : public QWidget,
                  public IGuiCmdHandler {
    Q_OBJECT
public:
    PnpWidget(IGui *igui, QWidget *parent);
    virtual ~PnpWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalUpdate();

private slots:
    void slotUpdate();

protected:
    virtual void showEvent(QShowEvent *event_);
    QLabel *getLabel(int id) {
        return static_cast<QLabel *>(mainLayout_->itemAt(id)->widget());
    }

private:
    QWidget *parent_;
    IGui *igui_;
    QGridLayout *mainLayout_;
    
    AttributeType lstSlaves_;
    enum ETargets {SIMULATION, ML605, KC705, TARGET_Unknown, TARGETS_total};
    QImage imgTarget_[TARGETS_total];

    PnpMapType pnp_;
    union DescriptorTableType {
        union DescriptorItemType {
            MasterConfigType mst;
            SlaveConfigType slv;
        } *item;
        uint8_t *buf;
    } iter_;
};

class PnpQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    PnpQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                      QAction *act)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("Plug'n'Play info"));
        setWindowIcon(QIcon(tr(":/images/board_96x96.png")));
        QWidget *pnew = new PnpWidget(igui, this);
        act->setChecked(true);
        setWidget(pnew);
        area_->addSubWindow(this);
        show();
    }
    
protected:
    void closeEvent(QCloseEvent *event_) Q_DECL_OVERRIDE {
        action_->setChecked(false);
        area_->removeSubWindow(this);
        event_->accept();
    }
private:
    QAction *action_;
    QMdiArea *area_;
};

}  // namespace debugger
