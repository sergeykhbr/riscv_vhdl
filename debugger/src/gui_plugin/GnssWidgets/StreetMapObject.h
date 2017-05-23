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

#include <QtCore/QUrl>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QHash>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkProxy>

namespace debugger {

class StreetMap: public QObject {
    Q_OBJECT

public:
    StreetMap(QObject *parent = 0, int zoom_=12);
    virtual ~StreetMap();

    void setZoom(int v) { zoom = v; }
    int getZoom() { return zoom; }
    // x = latitude; y = longitude
    void setCenterCoord(QPointF coord);
    QPointF getCenterCoord();
    QRectF getBorderCoord();
    QPoint coordToPixpos(QPointF coord);
    QRect coordToPixpos(QRectF coord);
    int getWidth() { return width; }
    int getHeight() { return height; }
    void resize(int w, int h);
    void render(QPainter *p, const QRect &rect);
    void pan(const QPoint &delta);


signals:
    void signalTilesUpdated(const QRect &rect);

private slots:
    void slotRequestNetworkData();
    void handleNetworkData(QNetworkReply *reply);
    void download();

protected:
    QRect tileRect(const QPoint &tp);

private:
    qreal latitude;
    qreal longitude;
    QPointF ct;
    int width;
    int height;
    int zoom;

    QPoint m_offset;
    QRect m_tilesRect;
    QPixmap m_emptyTile;
    QHash<QPoint, QPixmap> m_tilePixmaps;
    QNetworkAccessManager m_manager;
    QNetworkProxy m_proxy;
    QNetworkRequest m_request;
    QUrl m_url;
};

}  // namespace debugger
