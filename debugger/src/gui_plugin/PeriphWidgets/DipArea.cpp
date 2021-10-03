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

#include "DipArea.h"
#include <QtGui/QPainter>

namespace debugger {

DipArea::DipArea(QWidget *parent) : QWidget(parent) {
    dipTotal_.make_int64(8);
    dips_ = 0xF0;

    QImage img(tr(":/images/dip_t4_l21_up.png"));
    pixmapBody_ = QPixmap(size()).fromImage(img);

    QImage img2(tr(":/images/dip_down.png"));
    pixmapDown_ = QPixmap(size()).fromImage(img2);

    setMinimumWidth(pixmapBody_.size().width() * dipTotal_.to_int());
    setMinimumHeight(pixmapBody_.size().height());
}


void DipArea::paintEvent(QPaintEvent *event) {
    QPainter p(this);

    for (int i = 0; i < dipTotal_.to_int(); i++) {
        QPoint pos(i * pixmapBody_.size().width(), 0);
        p.drawPixmap(pos, pixmapBody_);
        // Count from left (DIP[0]) to right (DIP[n-1])
        if (((dips_ >> (dipTotal_.to_int() - 1 - i)) & 0x1) == 0) {
            pos += QPoint(4, 21);
            p.drawPixmap(pos, pixmapDown_);
        }
    }
    p.end();
}

void DipArea::slotUpdate(uint32_t value) {
    dips_ = value;
    update();
}

}  // namespace debugger
