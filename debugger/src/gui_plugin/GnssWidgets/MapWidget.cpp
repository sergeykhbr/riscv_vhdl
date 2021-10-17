/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <QtCore/QRandomGenerator>
#include <QtGui/QPainterPath>
#include "MapWidget.h"
#include <math.h>

namespace debugger {

/** Start marker of GNSS raw measurements that receiver generates in JSON
 *  format.
 */
const Reg64Type MAGIC_GNSS = {"{'Epoch"};

/** Test points that will shown even if no position availbale */
QPointF defaultPos[] = {
    {55.929967, 37.516868},     // MIPT, Dolgoprudniy (Moscow area)
    {37.871853, -122.258423},   // University of California, Berkely
    {59.336187, 18.068777}      // Sweden, Gaisler (Leon3) office location
};

MapWidget::MapWidget(IGui *igui, QWidget *parent)
    : QWidget(parent) {
    igui_ = igui;
    gnssIsParsing_ = false;
    bNewDataAvailable = true;
    pressed = false;
    invert = false;

    m_normalMap = new StreetMap(this, 17);
    m_miniMap = new StreetMap(this, 12);
    // This signal force redrawing when new data were downloaded
    connect(m_normalMap, SIGNAL(signalTilesUpdated(QRect)),
            this, SLOT(slotTilesUpdated(QRect)));   
    connect(m_miniMap, SIGNAL(signalTilesUpdated(QRect)),
            this, SLOT(slotTilesUpdated(QRect)));

    connect(this, SIGNAL(signalRequestNetworkData()),
            m_normalMap, SLOT(slotRequestNetworkData()));
    connect(this, SIGNAL(signalRequestNetworkData()),
            m_miniMap, SLOT(slotRequestNetworkData()));

    setFocusPolicy(Qt::ClickFocus);

    contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Clear"), this, SLOT(slotActionClear()));
    contextMenu->addAction(tr("Nightmode"), this, SLOT(slotActionNightMode()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
                  SLOT(slotRightClickMenu(const QPoint &)));   

    connect(this, SIGNAL(signalUpdateGnssRaw()),
                  SLOT(slotUpdateGnssRaw()));

    setWindowTitle(tr("Map"));

    posinfoSize = QSize(340, 200);
    posinfoPixmap = QPixmap(posinfoSize);

    const AttributeType *cfg = igui_->getpConfig();
    const AttributeType &serial_name = (*cfg)["Serial"];
    if (serial_name.is_string()) {
        uart_ = static_cast<ISerial *>
            (RISCV_get_service_iface(serial_name.to_string(), IFACE_SERIAL));
        if (uart_) {
            uart_->registerRawListener(static_cast<IRawListener *>(this));
        }
    }

    QDateTime sd = QDateTime::currentDateTime();
    QRandomGenerator rndgen;
    rndgen.seed(sd.toMSecsSinceEpoch());
    int pos_init_idx =
        rndgen.generate() % static_cast<int>(sizeof(defaultPos)/sizeof(QPointF));

#if 0
    pos_init_idx = 0;
    double lat = defaultPos[pos_init_idx].x() + (double)(rand() & 0x1f)/100000.0;
    double lon = defaultPos[pos_init_idx].y()+ (double)(rand() & 0x1f)/100000.0;
    for (int x = 0; x < 50; x++) {
        gpsLat_.put(lat);
        gpsLon_.put(lon);
        lat = defaultPos[pos_init_idx].x() + (double)(rand() & 0x1f)/100000.0;
        lon = defaultPos[pos_init_idx].y() + (double)(rand() & 0x1f)/100000.0;
    }
#endif
    m_normalMap->setCenterCoord(defaultPos[pos_init_idx]);
    m_miniMap->setCenterCoord(defaultPos[pos_init_idx]);
}

MapWidget::~MapWidget() {
    if (uart_) {
        uart_->unregisterRawListener(static_cast<IRawListener *>(this));
    }
}

int MapWidget::updateData(const char *buf, int buflen) {
    for (int i = 0; i < buflen; i++) {
        gnssMagicNumber_.buf[7] = buf[i];
        gnssMagicNumber_.val >>= 8;
        if (!gnssIsParsing_) {
            if (gnssMagicNumber_.val == MAGIC_GNSS.val) {
                memcpy(gnssBuf_, MAGIC_GNSS.buf, 8);
                gnssBufCnt_ = 7;
                gnssBraceCnt_ = 1;
                gnssIsParsing_ = true;
            }
            continue;
        }
        gnssBuf_[gnssBufCnt_++] = buf[i];
        gnssBuf_[gnssBufCnt_] = '\0';
        if (buf[i] == '{') {
            gnssBraceCnt_++;
            continue;
        } 
        if (buf[i] != '}') {
            continue;
        }
        if (--gnssBraceCnt_ == 0) {
            gnssIsParsing_ = false;
            gnssRawMeas_.from_config(gnssBuf_);
            emit signalUpdateGnssRaw();
        }
    }
    return buflen;
}

void MapWidget::slotUpdateGnssRaw() {
    if (!gnssRawMeas_.is_dict()) {
        return;
    }
    AttributeType &gps = gnssRawMeas_["GPS"];
    if (!gps.is_dict()) {
        return;
    }
    AttributeType &lms = gps["LMS"];
    if (!lms.is_list() || lms.size() < 8) {
        return;
    }
    if (lms[0u].to_int() == 0) {
        return;
    }
    double lat, lon;
    lat = static_cast<double>(lms[1].to_int());
    lat += lms[2].to_float() / 60.0;
    if (lms[3].is_equal("S")) {
        lat = -lat;
    }

    lon = static_cast<double>(lms[4].to_int());
    lon += lms[5].to_float() / 60.0;
    if (lms[6].is_equal("W")) {
        lon = -lon;
    }
    if (lat == 0 || lon == 0) {
        return;
    }
    gpsLat_.put(lat);
    gpsLon_.put(lon);

    if (!pressed) {
        QPointF coord(gpsLat_.get_avg(), gpsLon_.get_avg());
        m_normalMap->setCenterCoord(coord);
        m_miniMap->setCenterCoord(coord);
    }
    emit signalRequestNetworkData();
}


void MapWidget::slotTilesUpdated(QRect rect) {
    renderAll();
    update();
}

void MapWidget::slotActionClear() {
    //for (int i=0; i<DataTotal; i++) {
        //pPosTrack[i]->clear();
    //}
    gpsLat_.clear();
    gpsLon_.clear();
    renderAll();
    update();
}

void MapWidget::slotActionNightMode() {
    invert = !invert;
    renderAll();
    update();
}

void MapWidget::slotRightClickMenu(const QPoint &p) {
    QPoint globalPos = mapToGlobal(p);
    //contextMenu->exec(globalPos);
    contextMenu->popup(globalPos);
}

void MapWidget::resizeEvent(QResizeEvent *ev) {
    if (ev->size().height() == 0 || ev->size().width() == 0) {
        // Warning: When window inactive the height=0
        return;
    }
    mainmapSize = ev->size();
    int w = mainmapSize.width();
    int h = mainmapSize.height();
    m_normalMap->resize(w, h);


    int square_sz = qMin((2*w)/5, (2*h)/5);
    minimapSize = QSize(square_sz, square_sz);
    m_miniMap->resize(square_sz, square_sz);

    renderAll();
    update();

    if (bNewDataAvailable) {
        bNewDataAvailable = false;
        emit signalRequestNetworkData();
    }
}

void MapWidget::renderAll() {
    renderMinimap();
    renderMainMap();      // Rendering of the mainMap will cause update signal
}

void MapWidget::renderMainMap() {
    // only set the dimension to the magnified portion
    if (mainmapPixmap.size() != mainmapSize) {
        mainmapPixmap = QPixmap(mainmapSize);
        mainmapPixmap.fill(Qt::lightGray);
    }

    QPainter p_map(&mainmapPixmap);
    m_normalMap->render(&p_map, QRect(QPoint(0,0), mainmapSize));
    p_map.setPen(Qt::black);
    p_map.drawText(rect(),  Qt::AlignBottom | Qt::TextWordWrap,
                tr("Map data CCBYSA 2017 OpenStreetMap.org contributors"));

    // Draw Position track:
    p_map.translate(20,20);
    fontPos.setPixelSize(16);
    p_map.setFont(fontPos);

    renderTrack(0, p_map);

    p_map.end();
}

void MapWidget::renderTrack(int trkIdx, QPainter &p) {
    // Draw semi-transparent background for coordinates output:
    QColor trackLineColor(tr("#008F8F"));
    QColor trackPointColor(tr("#004848"));
    QColor trackTextColor;
    QPen pointPen(trackPointColor, 0, Qt::SolidLine, Qt::RoundCap);
    trackLineColor.setAlpha(0xa0);
    trackPointColor.setAlpha(0xa0);
    QPen linePen = QPen(trackLineColor, 0, Qt::SolidLine, Qt::RoundCap);

    p.setPen(pointPen);
    p.setRenderHint(QPainter::Antialiasing);

    QPoint xy;
    double lat = gpsLat_.getp()[0];
    double lon = gpsLon_.getp()[0];
    QPoint xy0 = m_normalMap->coordToPixpos(QPointF(lat, lon));
    p.drawLine(xy0.x() - 2, xy0.y(), xy0.x() + 2, xy0.y());
    p.drawLine(xy0.x(), xy0.y() + 2, xy0.x(), xy0.y() - 2);

    for (int i = 0; i < gpsLat_.size(); i++) {
        lat = gpsLat_.getp()[i];
        lon = gpsLon_.getp()[i];
        xy = m_normalMap->coordToPixpos(QPointF(lat, lon));
        p.setPen(linePen);
        p.drawLine(xy0.x(), xy0.y(), xy.x(), xy.y());
        xy0 = xy;

        p.setPen(pointPen);
        p.drawLine(xy0.x() - 2, xy0.y(), xy0.x() + 2, xy0.y());
        p.drawLine(xy0.x(), xy0.y() + 2, xy0.x(), xy0.y() - 2);
    }
    
    QString strPosition = QString::asprintf("GPS LMS: Lat %.4f; Lon %.4f", lat, lon);
    int h = p.fontMetrics().size(Qt::TextSingleLine, strPosition).height();

    QRect rect = QRect(QPoint(0, trkIdx * (h + 5)),
                       QPoint(340, (trkIdx + 1) * (h + 5)));
    p.fillRect(rect, QBrush(QColor(128, 128, 128, 128)));
    // Draw axis line template:
    int marginy = trkIdx*(h + 5)/2;
    int startx = 5;
    int starty = marginy + (h + 5)/2;
    int endx = 30;
    int endy = marginy + (h + 5)/2;
    p.setPen(linePen);
    p.drawLine(startx, starty, endx, endy);
    p.setPen(pointPen);
    p.drawLine(startx - 2, starty, startx + 2, starty);
    p.drawLine(startx, starty + 2, startx, starty - 2);
    p.drawLine(endx - 2, endy, endx + 2, endy);
    p.drawLine(endx, endy + 2, endx, endy - 2);

    trackTextColor.setRgb(0xff, 0xff, 0xff, 0xc0);
    p.setPen(trackTextColor);
    p.drawText(endx + 6, h + trkIdx*(h + 5) + 1, strPosition);
    trackTextColor.setRgb(0x20, 0x20, 0x20, 0xff);
    p.setPen(trackTextColor);
    p.drawText(endx + 5, h + trkIdx*(h + 5), strPosition);
}

void MapWidget::renderPosInfo(QPainter &p)
{
    QString info, strTmp;
    /*for (int i=0; i<POS_Total; i++) {
        if (PosTrack[i].ena == false) continue;
        strTmp.sprintf("%s: \n", PosTrack[i].name.c_str());
        info += strTmp;
    }*/

    QSize infoSize = p.fontMetrics().size(Qt::TextDontClip, info);
    QPoint infoPos = QPoint(10, 10);
    QRect infoRect = QRect(infoPos, infoSize);

    p.fillRect(infoRect, QBrush(QColor(0xff, 0xef, 0xd5, 0x80)));
    /*
    int sz = pPosTrack[type]->size();
    QColor clr = getDataColor(type);
    
    p.setPen(QPen(clr, 2, Qt::SolidLine, Qt::RoundCap));
    p.setRenderHint(QPainter::Antialiasing);
    */
}


void MapWidget::renderMinimap() {
    if (minimapSize.width() == 0 || minimapSize.height() == 0) {
        // At the beging there occurs strange resizeEvent()
        return;
    }

    // 1/2 x 1/2 of the main screen width:
    int w = mainmapSize.width();
    int h = mainmapSize.height();
    if (w == 0 || h == 0) 
        return;

    int square_sz = m_miniMap->getWidth();     // always square: height = width
    minimapRadius = square_sz/2;     //
    minimapInnerRadius = minimapRadius - 15;
    minimapSize = QSize(square_sz, square_sz);
    minimapPosition = QPoint(w - square_sz, h - square_sz);
    minimapCenter = minimapPosition + QPoint(minimapRadius, minimapRadius);

    
    // reupdate our mask
    if (minimapPixmapMask.size() != minimapSize) {
        minimapPixmapMask = QPixmap(minimapSize);
        minimapPixmapMask.fill(Qt::transparent);

        QRadialGradient g;
        g.setCenter(minimapRadius, minimapRadius);
        g.setFocalPoint(minimapRadius, minimapRadius);
        g.setRadius(minimapRadius);
        g.setColorAt(1.0, QColor(255, 255, 255, 0));
        g.setColorAt(0.5, QColor(128, 128, 128, 255));

        QPainter mask(&minimapPixmapMask);
        mask.setRenderHint(QPainter::Antialiasing);
        mask.setCompositionMode(QPainter::CompositionMode_Source);
        mask.setBrush(g);
        mask.setPen(Qt::NoPen);
        mask.drawRect(minimapPixmapMask.rect());
        mask.setBrush(QColor(Qt::transparent));
        mask.drawEllipse(g.center(), minimapInnerRadius, minimapInnerRadius);
        mask.end();
    }

    // only set the dimension to the magnified portion
    if (minimapPixmap.size() != minimapSize) {
        minimapPixmap = QPixmap(minimapSize);
        minimapPixmap.fill(Qt::lightGray);
    }
    QPainter p_map(&minimapPixmap);
    m_miniMap->render(&p_map, QRect(QPoint(0,0), minimapSize));

    // Draw small view area:
    p_map.setPen(Qt::red);
    QRectF border = m_normalMap->getBorderCoord();
    QRect view = m_miniMap->coordToPixpos(border);
    //view.translate(minimapPosition);
    p_map.drawLine(view.topLeft(), view.topRight());
    p_map.drawLine(view.topRight(), view.bottomRight());
    p_map.drawLine(view.bottomRight(), view.bottomLeft());
    p_map.drawLine(view.bottomLeft(), view.topLeft());

    p_map.end();
}


void MapWidget::paintEvent(QPaintEvent *event) {
    QPainter p;
    p.begin(this);

    // Main Map:
    p.drawPixmap(QPoint(0,0), mainmapPixmap);

    // Painting minimap:
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath clipPath;
    clipPath.addEllipse(minimapCenter, minimapInnerRadius, minimapInnerRadius);
    p.setClipPath(clipPath);
    p.drawPixmap(minimapPosition, minimapPixmap);
    p.setClipping(false);
    p.drawPixmap(minimapPosition, minimapPixmapMask);

    //renderPosInfo(p);

    p.setPen(Qt::gray);
    p.drawPath(clipPath);

    if (invert) {
        p.setCompositionMode(QPainter::CompositionMode_Difference);
        p.fillRect(event->rect(), Qt::white);
    }
    p.end();
}


void MapWidget::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    if (event->buttons() != Qt::LeftButton)
        return;
    pressed = true;
    pressPos = event->pos();
}

void MapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons())
        return;

    if (pressed) {
        QPoint delta = event->pos() - pressPos;
        pressPos = event->pos();
        m_normalMap->pan(delta);
        m_miniMap->setCenterCoord(m_normalMap->getCenterCoord());

        // Warning: 
        //      It will draw tiles that were already downloaded, others will be empty.
        //      Network request will on button release
        renderAll();
        update();
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent *) {
    pressed = false;
    emit signalRequestNetworkData();
}

void MapWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        if (m_normalMap->getZoom() < 19) {
            m_normalMap->setZoom(m_normalMap->getZoom()+1);
            m_normalMap->pan(QPoint());
            emit signalRequestNetworkData();
        }
        break;
    case Qt::Key_Minus:
        if (m_normalMap->getZoom() > 15) {
            m_normalMap->setZoom(m_normalMap->getZoom()-1);
            m_normalMap->pan(QPoint());
            emit signalRequestNetworkData();
        }
        break;
    default:;
    }
}

}  // namespace debugger
