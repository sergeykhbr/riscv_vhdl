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
 *
 * @brief      LED's area renderer definition.
 */

#include "LedArea.h"
#include <QtGui/QPainter>

namespace debugger {

LedArea::LedArea(QWidget *parent) : QWidget(parent) {
    ledTotal_.make_int64(8);
    leds_ = 0xF0;

    QImage img1(tr(":/images/led_on.png"));
    pixmapOn_ = QPixmap(size()).fromImage(img1);

    QImage img2(tr(":/images/led_off.png"));
    pixmapOff_ = QPixmap(size()).fromImage(img2);

    setMinimumWidth(pixmapOff_.size().width() * ledTotal_.to_int());
    setMinimumHeight(pixmapOff_.size().height());
}


void LedArea::paintEvent(QPaintEvent *event) {
    QPainter p(this);

    for (int i = 0; i < ledTotal_.to_int(); i++) {
        QPoint pos(i * pixmapOff_.width(), 0);

        // Count from left (LED[0]) to right (LED[n-1])
        if (((leds_ >> (ledTotal_.to_int() - 1 - i)) & 0x1) == 0) {
            p.drawPixmap(pos, pixmapOff_);
        } else {
            p.drawPixmap(pos, pixmapOn_);
        }
    }
    p.end();
}

void LedArea::slotUpdate(uint32_t value) {
    leds_ = value;
    update();
}

}  // namespace debugger
