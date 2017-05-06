/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Open Street map widget.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtCore/QBasicTimer>
#include <QtWidgets/QMenu>
#include <QtWidgets/QWidget>
#include <QtGui/qevent.h>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include "StreetMapObject.h"

namespace debugger {

class MapWidget : public QWidget,
                  public IGuiCmdHandler {
    Q_OBJECT
public:
    MapWidget(IGui *igui, QWidget *parent);
    virtual ~MapWidget();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalRequestNetworkData();
public slots:
    void slotMapUpdated(QRect);
    void slotRightClickMenu(const QPoint &p);
    //void slotEpochUpdated(EpochDataType *, GuiStorageType *);
    void slotActionClear();
    void slotActionNightMode();
    //void slotRepaintByTimer();
    //void slotAddPosition(Json &cfg);
    //void slotRemovePosition(Json &cfg);

protected:
    void setCentralPointByAverage();
    void renderAll();
    void renderMinimap();
    void renderPosInfo(QPainter &p);
    void renderMainMap();
    void renderTrack(int trkIdx, QPainter &p);

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *event);

private:
    bool bNewDataAvailable;

    StreetMap *m_normalMap;
    StreetMap *m_miniMap;
    bool pressed;
    QPoint pressPos;
    QFont fontPos;

    QSize mainmapSize;
    QPixmap mainmapPixmap;

    QSize posinfoSize;
    QPixmap posinfoPixmap;

    int minimapRadius;
    int minimapInnerRadius;
    QSize minimapSize;
    QPoint minimapPosition;
    QPoint minimapCenter;
    QPixmap minimapPixmap;
    QPixmap minimapPixmapMask;
    QMenu *contextMenu;
    bool invert;

    int TrackHistory;
    /*struct TrackType {
        bool ena;
        LineCommon *lineLat;
        LineCommon *lineLon;
        QColor color;
        std::string name;
        int used;
    } PosTrack[POS_Total];*/
};

class MapQMdiSubWindow : public QMdiSubWindow {
    Q_OBJECT
public:
    MapQMdiSubWindow(IGui *igui, QMdiArea *area, QWidget *parent,
                      QAction *act)
        : QMdiSubWindow(parent) {
        setAttribute(Qt::WA_DeleteOnClose);
        action_ = act;
        area_ = area;

        setWindowTitle(tr("Map"));
        setMinimumWidth(parent->size().width() / 2);
        QWidget *pnew = new MapWidget(igui, this);
        setWindowIcon(act->icon());
        act->setChecked(true);
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

