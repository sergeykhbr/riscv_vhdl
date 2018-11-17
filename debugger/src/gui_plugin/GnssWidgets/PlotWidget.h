/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Generic Plot drawer widget.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "linecommon.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QMenu>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/qevent.h>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QVBoxLayout>

namespace debugger {

static const int LINES_PER_PLOT_MAX = 8;

class PlotWidget : public QWidget,
                   public IGuiCmdHandler {
    Q_OBJECT

public:
    PlotWidget(IGui *igui, QWidget *parent = 0);
    virtual ~PlotWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

public slots:
    void slotUpdateByTimer();
    virtual void slotCmdResponse() =0;
    void slotRightClickMenu(const QPoint &p);
    void slotActionZoomClear();

signals:
    void signalCmdResponse();

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    void keyPressEvent(QKeyEvent *event);

private:
    void renderAll();
    void renderAxis(QPainter &p);
    void renderLine(QPainter &p, LineCommon *pline);
    void renderMarker(QPainter &p);
    void renderSelection(QPainter &p);
    void renderInfoPanel(QPainter &p);
    int pix2epoch(QPoint pix);

    double borderUpValue(double v);

protected:
    IGui *igui_;
    AttributeType cmd_;
    AttributeType response_;
    AttributeType defaultLineCfg;

    QColor bkg1;

    bool waitingResp_;
    Qt::MouseButton pressed;
    QPoint pressStart;          /** Mouse Middle button start coordinates */
    QPoint pressEnd;            /** Mouse Middle button end coordinates */

    int epochStart;             /** Draw data starting from this index  */
    int epochTotal;             /** Draw the following number of epochs */
    int selectedEpoch;

    double dmax;
    double dmin;
    QPixmap pixmap;

    int lineTotal;
    int trackLineIdx;
    LineCommon *line_[LINES_PER_PLOT_MAX];
    QString groupName;
    QString groupUnits;

    QMenu *contextMenu;
    QRect rectMargined;
    QRect rectPlot;
};

class CpiPlot : public PlotWidget {
public:
    CpiPlot(IGui *igui, QWidget *parent = 0);
    virtual void slotCmdResponse();
};

class BusUtilPlot : public PlotWidget {
public:
    BusUtilPlot(IGui *igui, QWidget *parent = 0);
    virtual void slotCmdResponse();
};


class PlotQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    PlotQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                      QAction *act)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(act->text());
        setMinimumWidth(400);
        setMinimumHeight(280);
        QWidget *pnew1 = new CpiPlot(igui, this);
        connect(parent, SIGNAL(signalUpdateByTimer()),
                pnew1, SLOT(slotUpdateByTimer()));

        QWidget *pnew2 = new BusUtilPlot(igui, this);
        connect(parent, SIGNAL(signalUpdateByTimer()),
                pnew2, SLOT(slotUpdateByTimer()));

        setWindowIcon(act->icon());
        act->setChecked(true);

        layout()->setSpacing(2);
        layout()->addWidget(pnew1);
        layout()->addWidget(pnew2);
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
