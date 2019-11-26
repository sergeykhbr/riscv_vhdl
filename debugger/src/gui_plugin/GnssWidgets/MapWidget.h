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
#include "coreservices/irawlistener.h"
#include "coreservices/iserial.h"

#include <QtCore/QBasicTimer>
#include <QtWidgets/QMenu>
#include <QtWidgets/QWidget>
#include <QtGui/qevent.h>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include "StreetMapObject.h"

namespace debugger {

class SlideAverageType {
public:
    SlideAverageType(int sz = 32) {
        size_ = sz;
        data_ = new double [2*sz];
        clear();
    }
    ~SlideAverageType() {
        delete [] data_;
    }
    void put(double v) {
        if (--pcur_ < data_) {
            pcur_ = &data_[size_ - 1];
        }
        avg_sum_ -= *pcur_;
        *pcur_ = *(pcur_ + size_) = v;
        avg_sum_ += v;
        if (avg_cnt_ < size_) {
            avg_cnt_++;
        }
    }
    double *getp() {
        return pcur_;
    }
    double get_avg() {
        return avg_cnt_ == 0 ? 0 : avg_sum_/avg_cnt_;
    }
    int size() {
        return avg_cnt_;
    }
    void clear() {
        pcur_ = &data_[size_];
        avg_cnt_ = 0;
        avg_sum_ = 0;
        memset(data_, 0, 2*size_*sizeof(double));
    }
private:
    int size_;
    double *data_;
    double *pcur_;
    double avg_cnt_;
    double avg_sum_;
};

class MapWidget : public QWidget,
                  public IRawListener {
    Q_OBJECT
public:
    MapWidget(IGui *igui, QWidget *parent);
    virtual ~MapWidget();

    /** IRawListener */
    virtual int updateData(const char *buf, int buflen);

signals:
    void signalRequestNetworkData();
    void signalUpdateGnssRaw();
public slots:
    void slotUpdateGnssRaw();
    void slotTilesUpdated(QRect);
    void slotRightClickMenu(const QPoint &p);
    void slotActionClear();
    void slotActionNightMode();

protected:
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
    IGui *igui_;
    ISerial *uart_;

    // Gnss parser's data
    char gnssBuf_[1 << 16];
    int gnssBufCnt_;
    int gnssBraceCnt_;
    Reg64Type gnssMagicNumber_;
    bool gnssIsParsing_;
    AttributeType gnssRawMeas_;


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

    SlideAverageType gpsLat_;
    SlideAverageType gpsLon_;
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
        setMinimumHeight(parent->size().height() / 2);
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

