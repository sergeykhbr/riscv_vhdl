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

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtGui/QAction>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>

namespace debugger {

class LedDisplay : public QWidget,
                   public IGuiCmdHandler {
    Q_OBJECT
 public:
    LedDisplay(IGui *gui, QWidget *parent, const char *objname);
    virtual ~LedDisplay();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

 public slots:
    void slotUpdateByTimer();
    void slotOpenContextMenu(const QPoint &pos);
    void slotActionMakeScreenshot(bool);

 protected slots:
    void slotConfigurate();
    void slotHandleResponse();

 signals:
    void signalConfigurate();
    void signalHandleResponse();

 protected:
    void paintEvent(QPaintEvent *event_) Q_DECL_OVERRIDE;

 private:
    struct FrameItemType {
        int x;
        int y;
        uint32_t rgb;
    };
    void drawPixel(QPainter *p, FrameItemType *pix, int scale);

private:
    IGui *igui_;
    AttributeType cmdconfig_;
    AttributeType cmdframe_;
    AttributeType respConfig_;
    AttributeType respFrame_;

    QAction *actionScreenshot_;
    QPixmap pixmap_;
    QPixmap pixmapScreenshot_;

    int width_;
    int height_;
    int scale_;
    int screenshot_scale_;

    bool requested_;
};

}  // namespace debugger
