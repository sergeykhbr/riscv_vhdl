#include "MapWidget.h"
#include "moc_MapWidget.h"
#include <math.h>

namespace debugger {

MapWidget::MapWidget(IGui *igui, QWidget *parent)
    : QWidget(parent) {
    bNewDataAvailable = true;
    pressed = false;
    invert = false;

    m_normalMap = new StreetMap(this, 17);
    m_miniMap = new StreetMap(this, 12);
    // This signal force redrawing when new data were download
    connect(m_normalMap, SIGNAL(updated(QRect)), SLOT(slotMapUpdated(QRect)));   
    //connect(m_miniMap, SIGNAL(updated(QRect)), SLOT(slotMapUpdated(QRect)));

    connect(this, SIGNAL(signalRequestNetworkData()), m_normalMap, SLOT(slotRequestNetworkData()));
    connect(this, SIGNAL(signalRequestNetworkData()), m_miniMap, SLOT(slotRequestNetworkData()));

    setFocusPolicy(Qt::ClickFocus);

    contextMenu = new QMenu(this);
    contextMenu->addAction(tr("Clear"), this, SLOT(slotActionClear()));
    contextMenu->addAction(tr("Nightmode"), this, SLOT(slotActionNightMode()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotRightClickMenu(const QPoint &)));   

    setWindowTitle(tr("Map"));

    posinfoSize = QSize(340, 200);
    posinfoPixmap = QPixmap(posinfoSize);

    //memset(&PosTrack, 0, sizeof(PosTrack));
}

MapWidget::~MapWidget() {
}

void MapWidget::handleResponse(AttributeType *req, AttributeType *resp) {
}

void MapWidget::setCentralPointByAverage() {
    double avgLat = 0, avgLon;
    /*int start;
    if (PosTrack[POS_RMC].ena) {
        start = PosTrack[POS_RMC].lineLat->size() - TrackHistory;
        if (start < 0) {
            start = 0;
        }
        PosTrack[POS_RMC].lineLat->getAverageOf(start, TrackHistory, avgLat);
        PosTrack[POS_RMC].lineLon->getAverageOf(start, TrackHistory, avgLon);
    }
    else if (PosTrack[POS_GPSLMS].ena) {
        start = PosTrack[POS_GPSLMS].lineLat->size() - TrackHistory;
        if (start < 0) {
            start = 0;
        }
        PosTrack[POS_GPSLMS].lineLat->getAverageOf(start, TrackHistory, avgLat);
        PosTrack[POS_GPSLMS].lineLon->getAverageOf(start, TrackHistory, avgLon);
    }
    else if (PosTrack[POS_GLOLMS].ena) {
        start = PosTrack[POS_GLOLMS].lineLat->size() - TrackHistory;
        if (start < 0) {
            start = 0;
        }
        PosTrack[POS_GLOLMS].lineLat->getAverageOf(start, TrackHistory, avgLat);
        PosTrack[POS_GLOLMS].lineLon->getAverageOf(start, TrackHistory, avgLon);
    }
    else if (PosTrack[POS_LMSAVG].ena) {
        start = PosTrack[POS_LMSAVG].lineLat->size() - TrackHistory;
        if (start < 0) {
            start = 0;
        }
        PosTrack[POS_LMSAVG].lineLat->getAverageOf(start, TrackHistory, avgLat);
        PosTrack[POS_LMSAVG].lineLon->getAverageOf(start, TrackHistory, avgLon);
    }*/

    if (avgLat) {
        QPointF coord(avgLat, avgLon);
        m_normalMap->setCenterCoord(coord);
        m_miniMap->setCenterCoord(coord);
    }
}


/*void MapWidget::slotAddPosition(Json &cfg)
{
    int posIdx = -1;
    if (cfg["Name"].getString() == "RMC") {
        posIdx = POS_RMC;
    } else if (cfg["Name"].getString() == "GPS LMS") {
        posIdx = POS_GPSLMS;
    } else if (cfg["Name"].getString() == "GLO LMS") {
        posIdx = POS_GLOLMS;
    } else if (cfg["Name"].getString() == "LMS avg") {
        posIdx = POS_LMSAVG;
    }

    if (posIdx == -1)
        return;

    PosTrack[posIdx].ena = true;
    PosTrack[posIdx].lineLat = (LineCommon *)cfg["Lat"].getData();
    PosTrack[posIdx].lineLon = (LineCommon *)cfg["Lon"].getData();
    PosTrack[posIdx].color = QColor(cfg["Color"].getString().c_str());
    PosTrack[posIdx].name = cfg["Name"].getString();

    setCentralPointByAverage();

    renderAll();
    update();
}

void MapWidget::slotRemovePosition(Json &cfg)
{
    int posIdx = -1;
    if (cfg["Name"].getString() == "RMC") {
        posIdx = POS_RMC;
    } else if (cfg["Name"].getString() == "GPS LMS") {
        posIdx = POS_GPSLMS;
    } else if (cfg["Name"].getString() == "GLO LMS") {
        posIdx = POS_GLOLMS;
    } else if (cfg["Name"].getString() == "LMS avg") {
        posIdx = POS_LMSAVG;
    }

    if (posIdx == -1)
        return;

    PosTrack[posIdx].ena = false;
    renderAll();
    update();
}*/

void MapWidget::slotMapUpdated(QRect rect)
{
    renderAll();
    update();
}

/*void MapWidget::slotRepaintByTimer()
{
    if (bNewDataAvailable) {
        bNewDataAvailable = false;
        emit signalRequestNetworkData();
    }
}*/

void MapWidget::slotActionClear()
{
    //for (int i=0; i<DataTotal; i++) {
        //pPosTrack[i]->clear();
    //}
    renderAll();
    update();
}

void MapWidget::slotActionNightMode()
{
    invert = !invert;
    renderAll();
    update();
}

void MapWidget::slotRightClickMenu(const QPoint &p)
{
    QPoint globalPos = mapToGlobal(p);
    //contextMenu->exec(globalPos);
    contextMenu->popup(globalPos);
}

/*void MapWidget::slotEpochUpdated(EpochDataType *pEpoch, GuiStorageType *pStorage)
{
    if (pEpoch->posTotal == 0)
        return;

    if (!pressed) {
        setCentralPointByAverage();
    }
    // WARNING: 
    //        We may call this methods several time at once, so wait timer event
    //        to request updates:
    bNewDataAvailable = true;
}
*/
void MapWidget::resizeEvent(QResizeEvent *ev)
{
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
}

void MapWidget::renderAll()
{
    renderMinimap();
    renderMainMap();      // Rendering of the mainMap will cause update signal
}

void MapWidget::renderMainMap()
{
    // only set the dimension to the magnified portion
    if (mainmapPixmap.size() != mainmapSize) {
        mainmapPixmap = QPixmap(mainmapSize);
        mainmapPixmap.fill(Qt::lightGray);
    }

    QPainter p_map(&mainmapPixmap);
    m_normalMap->render(&p_map, QRect(QPoint(0,0), mainmapSize));
    p_map.setPen(Qt::black);
    p_map.drawText(rect(),  Qt::AlignBottom | Qt::TextWordWrap,
                "Map data CCBYSA 2017 OpenStreetMap.org contributors");

    // Draw Position track:
    p_map.translate(20,20);
    fontPos.setPixelSize(16);
    p_map.setFont(fontPos);

    /*int trkIdx = 0;
    for (int i=0; i<POS_Total; i++) {
        if (!PosTrack[i].ena)
             continue;

        renderTrack(i, p_map);
    }*/

    p_map.end();
}

void MapWidget::renderTrack(int trkIdx, QPainter &p)
{
/*
    // Draw semi-transparent background for coordinates output:
    TrackType *pos = &PosTrack[trkIdx];
    int sz = pos->lineLat->size();
    int start = 0;
    if (sz > TrackHistory) {
        start = sz - TrackHistory;
        sz = TrackHistory;
    }

    QColor trackColor(pos->color);
    QPen pointPen(trackColor, 0, Qt::SolidLine, Qt::RoundCap);
    trackColor.setAlpha(0xa0);
    QPen linePen = QPen(trackColor, 0, Qt::SolidLine, Qt::RoundCap);

    p.setPen(pointPen);
    p.setRenderHint(QPainter::Antialiasing);

    QPoint xy;
    double lat, lon;
    pos->lineLat->getDoubleByIdx(start, lat);
    pos->lineLon->getDoubleByIdx(start, lon);
    QPoint xy0 = m_normalMap->coordToPixpos(QPointF(lat, lon));
    p.drawLine(xy0.x() - 2, xy0.y(), xy0.x() + 2, xy0.y());
    p.drawLine(xy0.x(), xy0.y()+2, xy0.x(), xy0.y()-2);

    for (int i=start+1; i<start+sz; i++) {
        pos->lineLat->getDoubleByIdx(i, lat);
        pos->lineLon->getDoubleByIdx(i, lon);
        xy = m_normalMap->coordToPixpos(QPointF(lat, lon));
        p.setPen(linePen);
        p.drawLine(xy0.x(), xy0.y(), xy.x(), xy.y());
        xy0 = xy;

        p.setPen(pointPen);
        p.drawLine(xy0.x() - 2, xy0.y(), xy0.x() + 2, xy0.y());
        p.drawLine(xy0.x(), xy0.y() + 2, xy0.x(), xy0.y() - 2);
    }
    
    QString strPosition;
    strPosition.sprintf("%8s: Lat %.4f; Lon %.4f (%d)", pos->name.c_str(), lat, lon, pos->used);
    int h = p.fontMetrics().size(Qt::TextSingleLine, strPosition).height();

    QRect rect = QRect(QPoint(0,trkIdx*(h+5)), QPoint(340, (trkIdx+1)*(h+5)));
    p.fillRect(rect, QBrush(QColor(128, 128, 128, 128)));
    p.drawLine(0, trkIdx*(h+5) + h/2, 30, trkIdx*(h+5) + h/2);
    p.drawText(35, h + trkIdx*(h+5), strPosition);
    */
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


void MapWidget::renderMinimap()
{
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


void MapWidget::paintEvent(QPaintEvent *event)
{
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

void MapWidget::mouseReleaseEvent(QMouseEvent *)
{
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
