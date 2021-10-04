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

#include "StreetMapObject.h"
#include <QtWidgets/QtWidgets>
#include <QtCore/QDateTime>
#include <QtNetwork/QtNetwork>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace debugger {

// tile size in pixels
static const int TILE_SIZE = 256;

static QPointF tileForCoordinate(qreal lat, qreal lng, int zoom) {
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal tx = (lng + 180.0) / 360.0;
    qreal ty = (1.0 - log(tan(lat * M_PI / 180.0) +
                          1.0 / cos(lat * M_PI / 180.0)) / M_PI) / 2.0;
    return QPointF(tx * zn, ty * zn);
}

static qreal longitudeFromTile(qreal tx, int zoom) {
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal lat = tx / zn * 360.0 - 180.0;
    return lat;
}

static qreal latitudeFromTile(qreal ty, int zoom) {
    qreal zn = static_cast<qreal>(1 << zoom);
    qreal n = M_PI - 2 * M_PI * ty / zn;
    qreal lng = 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
    return lng;
}


// Zoom: 1=world map
//       12 = 0.5 of Moscow
//       15 = default
StreetMap::StreetMap(QObject *parent, int zoom_)
    : QObject(parent) {
    width = 400;
    height = 300;
    zoom = zoom_;

    latitude = 0;
    longitude = 0;

    m_request.setRawHeader("User-Agent", "sergeykhbr (RISC-V debugger)");
    m_emptyTile = QPixmap(TILE_SIZE, TILE_SIZE);
    m_emptyTile.fill(Qt::lightGray);

#if 0
    m_proxy.setType(QNetworkProxy::HttpProxy);
    m_proxy.setHostName(tr("http://proxy.server.com"));
    m_proxy.setPort(911);
    m_manager.setProxy(m_proxy);
#endif

    QNetworkDiskCache *cache = new QNetworkDiskCache;
    cache->setCacheDirectory(
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    m_manager.setCache(cache);
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));
}

StreetMap::~StreetMap() {
}

QPoint StreetMap::coordToPixpos(QPointF coord) {
    QPointF f = tileForCoordinate(coord.x(), coord.y(), zoom);
    f -= ct;
    f *= TILE_SIZE;
    int x = width/2 + (int)(f.x() + 0.5);
    int y = height/2 + (int)((int)f.y() + 0.5);
    return QPoint(x, y);
}

QRect StreetMap::coordToPixpos(QRectF coord) {
    QRect ret;
    QPoint c = coordToPixpos(coord.topLeft());
    ret.setTopLeft(c);
    c = coordToPixpos(coord.bottomRight());
    ret.setBottomRight(c);
    return ret;
}

QRectF StreetMap::getBorderCoord() {
    QRectF ret;
    qreal x0 = ct.x() - 0.5*double(width)/TILE_SIZE;
    qreal x1 = ct.x() + 0.5*double(width)/TILE_SIZE;
    ret.setTop(longitudeFromTile(x0, zoom));
    ret.setBottom(longitudeFromTile(x1, zoom));

    qreal y0 = ct.y() - 0.5*double(height)/TILE_SIZE;
    qreal y1 = ct.y() + 0.5*double(height)/TILE_SIZE;
    ret.setLeft(latitudeFromTile(y0, zoom));
    ret.setRight(latitudeFromTile(y1, zoom));
    return ret;
}


void StreetMap::setCenterCoord(QPointF coord) {
    if (width <= 0 || height <= 0)
        return;

    latitude = coord.x();
    longitude = coord.y();

    // Normalize lat/lon to range (0.0...1.0)*(1<<scale)
    // It means that full world map will be splitted on (1<<scale) x (1<<scale) rectangles.
    // If scale =12, then full map 4096 x 4096 tiles
    ct = tileForCoordinate(latitude, longitude, zoom);
    qreal tx = ct.x();
    qreal ty = ct.y();

    // Each tile 256x256 size:
    double centertile_x = (tx - (double)((int)tx)) * TILE_SIZE;
    double centertile_y = (ty - (double)((int)ty)) * TILE_SIZE;

    // top-left corner of the center tile
    int xp = width / 2 - centertile_x;
    int yp = height / 2 - centertile_y;

    // first tile vertical and horizontal
    int beforetile_x = (xp + TILE_SIZE - 1) / TILE_SIZE;
    int beforetile_y = (yp + TILE_SIZE - 1) / TILE_SIZE;
    int xs = static_cast<int>(tx) - beforetile_x;
    int ys = static_cast<int>(ty) - beforetile_y;


    // offset for top-left tile
    m_offset = QPoint(xp - beforetile_x * TILE_SIZE,
                      yp - beforetile_y * TILE_SIZE);

    // last tile vertical and horizontal
    int aftertile_x = (width - xp - 1) / TILE_SIZE;
    int aftertile_y = (height - yp - 1) / TILE_SIZE;
    int xe = static_cast<int>(tx) + aftertile_x;
    int ye = static_cast<int>(ty) + aftertile_y;

    // build a rect
    m_tilesRect = QRect(xs, ys, xe - xs + 1, ye - ys + 1);
}

QPointF StreetMap::getCenterCoord() {
    return QPointF(latitude, longitude);
}

void StreetMap::resize(int w, int h) {
    width = w;
    height = h;
    setCenterCoord(QPointF(latitude, longitude));
}

void StreetMap::render(QPainter *p, const QRect &rect) {
    for (int x = 0; x <= m_tilesRect.width(); ++x) {
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
            QPoint tp(x + m_tilesRect.left(), y + m_tilesRect.top());
            QRect box = tileRect(tp);
            if (rect.intersects(box)) {
                if (m_tilePixmaps.contains(tp)) {
                    p->drawPixmap(box, m_tilePixmaps.value(tp));
                } else {
                    p->drawPixmap(box, m_emptyTile);
                }
            }
        }
    }
}

void StreetMap::pan(const QPoint &delta) {
    QPointF dx = QPointF(delta) / qreal(TILE_SIZE);
    QPointF center = tileForCoordinate(latitude, longitude, zoom) - dx;
    qreal lat = latitudeFromTile(center.y(), zoom);
    qreal lon = longitudeFromTile(center.x(), zoom);
    setCenterCoord(QPointF(lat, lon));
}

void StreetMap::slotRequestNetworkData() {
    download();
}

void StreetMap::handleNetworkData(QNetworkReply *reply) {
    QImage img;
    QPoint tp = reply->request().attribute(QNetworkRequest::User).toPoint();
    QUrl url = reply->url();
    if (!reply->error()) {
        if (!img.load(reply, 0)) {
            img = QImage();
        }
    }
    reply->deleteLater();
    if (img.isNull()) {
        m_tilePixmaps[tp] = m_emptyTile;
    } else {
        m_tilePixmaps[tp] = QPixmap::fromImage(img);
    }

    emit signalTilesUpdated(tileRect(tp));

    // purge unused spaces
    QRect bound = m_tilesRect.adjusted(-2, -2, 2, 2);
    foreach(QPoint tp, m_tilePixmaps.keys())
    if (!bound.contains(tp)) {
        m_tilePixmaps.remove(tp);
    }

    download();
}

void StreetMap::download() {
    QPoint grab(0, 0);
    for (int x = 0; x <= m_tilesRect.width(); ++x) {
        for (int y = 0; y <= m_tilesRect.height(); ++y) {
            QPoint tp = m_tilesRect.topLeft() + QPoint(x, y);
            if (!m_tilePixmaps.contains(tp)) {
                grab = tp;
                break;
            }
        }
    }
    if (grab == QPoint(0, 0)) {
        m_url = QUrl();
        emit signalTilesUpdated(QRect());
        return;
    }

    QString path = "http://tile.openstreetmap.org/%1/%2/%3.png";
    m_url = QUrl(path.arg(zoom).arg(grab.x()).arg(grab.y()));
    m_request.setUrl(m_url);
    QVariant arg(grab);
    m_request.setAttribute(QNetworkRequest::User, arg);
    m_manager.get(m_request);
}

QRect StreetMap::tileRect(const QPoint &tp) {
    QPoint t = tp - m_tilesRect.topLeft();
    int x = t.x() * TILE_SIZE + m_offset.x();
    int y = t.y() * TILE_SIZE + m_offset.y();
    return QRect(x, y, TILE_SIZE, TILE_SIZE);
}

}  // namespace debugger
