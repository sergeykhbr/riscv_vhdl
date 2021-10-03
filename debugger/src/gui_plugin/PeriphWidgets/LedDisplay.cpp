/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#include "LedDisplay.h"
#include <memory>
#include <string.h>
#include <QtCore/QFile>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>

namespace debugger {

LedDisplay::LedDisplay(IGui *gui, QWidget *parent, const char *objname) 
    : QWidget(parent) {
    igui_ = gui;

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s config", objname);
    cmdconfig_.make_string(tstr);

    RISCV_sprintf(tstr, sizeof(tstr), "%s frame diff", objname);
    cmdframe_.make_string(tstr);

    const AttributeType &cfgDisplay = 
        (*igui_->getpConfig())["DemoM4Widgets"]["Display"];

    width_ = cfgDisplay["Width"].to_int();
    height_ = cfgDisplay["Height"].to_int();
    scale_ = cfgDisplay["Scale"].to_int();
    setFixedHeight(scale_*height_ - 1);
    setFixedWidth(scale_*width_ - 1);

    if (scale_ <= 0) {
        scale_ = 1;
    }
    screenshot_scale_ = cfgDisplay["ScreenShotScale"].to_int();
    if (screenshot_scale_ <= 0) {
        screenshot_scale_ = 1;
    }

    connect(this, SIGNAL(signalConfigurate()), SLOT(slotConfigurate()));
    connect(this, SIGNAL(signalHandleResponse()), SLOT(slotHandleResponse()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), 
            this, SLOT(slotOpenContextMenu(const QPoint &)));

    actionScreenshot_ = new QAction("Screenshot", this);
    actionScreenshot_->setCheckable(false);
    connect(actionScreenshot_, SIGNAL(triggered(bool)),
            this, SLOT(slotActionMakeScreenshot(bool)));

    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           cmdconfig_.to_string(), &respConfig_, true);
    requested_ = true;
}

LedDisplay::~LedDisplay() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void LedDisplay::handleResponse(const char *cmd) {
    if (strcmp(cmd, cmdconfig_.to_string()) == 0) {
        emit signalConfigurate();
    } else if (strcmp(cmd, cmdframe_.to_string()) == 0) {
        if (respFrame_.is_data()) {
            emit signalHandleResponse();
        } else {
            requested_ = false;
        }
    }
}

void LedDisplay::slotConfigurate() {
    width_ = respConfig_["Width"].to_int();
    height_ = respConfig_["Height"].to_int();

    setFixedHeight(scale_*height_ - 1);
    setFixedWidth(scale_*width_ - 1);

    pixmap_ = QPixmap(QSize(scale_*width_-1, scale_*height_-1));
    pixmap_.fill(QColor(respConfig_["BkgColor"].to_uint32()));

    pixmapScreenshot_ = QPixmap(QSize(screenshot_scale_*width_,
                                screenshot_scale_*height_));
    pixmapScreenshot_.fill(QColor(respConfig_["BkgColor"].to_uint32()));

    RISCV_memory_barrier();
    requested_ = false;
}

void LedDisplay::slotUpdateByTimer() {
    if (requested_) {
        return;
    }
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                          cmdframe_.to_string(), &respFrame_, true);
    requested_ = true;
}

void LedDisplay::slotHandleResponse() {
    QPainter p(&pixmap_);
    QPainter p2(&pixmapScreenshot_);
    p.setRenderHint(QPainter::Antialiasing, false);
    p2.setRenderHint(QPainter::Antialiasing, false);

    uint32_t *pframe = reinterpret_cast<uint32_t *>(respFrame_.data());
    unsigned sz = respFrame_.size() / sizeof(uint32_t);
    FrameItemType pix;
    for (unsigned i = 0; i < sz; i++) {
        pix.x = i / height_;
        pix.y = i - height_ * pix.x;
        pix.rgb = pframe[i];
        drawPixel(&p, &pix, scale_);
        drawPixel(&p2, &pix, screenshot_scale_);
    }
    p2.end();
    p.end();
    update();

    RISCV_memory_barrier();
    requested_ = false;
}

void LedDisplay::drawPixel(QPainter *p, FrameItemType *pix, int scale) {
    p->setPen(QPen(QColor(pix->rgb)));
    p->setBrush(QBrush(QColor(pix->rgb)));
    if (scale >= 3) {
        QRect pos(0, 0, scale-2, scale-2);
        pos.moveLeft(scale * pix->x);
        pos.moveTop(scale * pix->y);
        p->drawRect(pos);
    } else {
        p->drawPoint(scale*pix->x, scale*pix->y);
    }
}

void LedDisplay::paintEvent(QPaintEvent *event_) {
    QPainter p(this);
    QPoint pos(0,0);
    p.drawPixmap(pos, pixmap_);
    p.end();
}

void LedDisplay::slotOpenContextMenu(const QPoint &pos) {
   QMenu contextMenu(tr("Context menu"), this);

   contextMenu.addAction(actionScreenshot_);
   contextMenu.exec(mapToGlobal(pos));
}

void LedDisplay::slotActionMakeScreenshot(bool trig) {
    const AttributeType &cfgDisplay = 
        (*igui_->getpConfig())["CmeWidgets"]["Display"];
    QString fileName = QFileDialog::getSaveFileName(this,
                tr("Save screenshot"),
                tr(cfgDisplay["SaveFolder"].to_string()),
                tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName == "") {
        return;
    }

    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    pixmapScreenshot_.save(&file, "PNG");
}

}  // namespace debugger
