/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      LED's area renderer definition.
 */

#include "LedArea.h"
#include "moc_LedArea.h"

#include <QtGui/QPainter>

namespace debugger {

LedArea::LedArea(QWidget *parent) : QWidget(parent) {
    setWindowTitle(tr("gpio0"));
    setMinimumWidth(150);
    setMinimumHeight(100);
    QImage img(tr(":/images/toggle.png"));
    pixmapBkg_ = QPixmap(size()).fromImage(img);
    pixmapGreen_ = QPixmap(QSize(6, 10));
    pixmapGreen_ .fill(QColor("#10A010"));
    pixmapGrey_ = QPixmap(QSize(6, 10));
    pixmapGrey_ .fill(QColor("#101010"));
    leds_ = 0xFF;
}


void LedArea::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.drawPixmap(QPoint(0, 0), pixmapBkg_);
    for (int i = 0; i < 8; i++) {
        if (leds_ & (1ul << i)) {
            p.drawPixmap(QPoint(10 + 14*i, 20), pixmapGreen_);
        } else {
            p.drawPixmap(QPoint(10 + 14*i, 20), pixmapGrey_);
        }
    }
    p.end();
}

void LedArea::slotUpdate(uint32_t value) {
    leds_ = value;
    update();
}

}  // namespace debugger
