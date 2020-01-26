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
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/qevent.h>

namespace debugger {

class DemoSTM32Keypad : public QWidget,
                        public IGuiCmdHandler {
    Q_OBJECT
 public:
    DemoSTM32Keypad(QWidget *parent, IGui *igui);
    virtual ~DemoSTM32Keypad();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

    enum EBtnNames {
        KEY_1,
        KEY_2,
        KEY_3,
        KEY_4,
        KEY_5,
        KEY_6,
        KEY_7,
        KEY_8,
        KEY_9,
        KEY_0,
        KEY_P1,
        KEY_P2,
        KEY_P3,
        KEY_P4,
        KEY_P5,
        KEY_P6,
        KEY_P7,
        KEY_Total
    };

 protected slots:
    void slotUpdateByTimer();
    void slotHandleResponse();
    void slotExternalCommand(const char *cmd);
    void slotBtn1Toggled(bool val);
    void slotBtn2Toggled(bool val);
    void slotBtn3Toggled(bool val);
    void slotBtn4Toggled(bool val);
    void slotBtn5Toggled(bool val);
    void slotBtn6Toggled(bool val);
    void slotBtn7Toggled(bool val);
    void slotBtn8Toggled(bool val);
    void slotBtn9Toggled(bool val);
    void slotBtn0Toggled(bool val);
    void slotBtnP1Toggled(bool val);
    void slotBtnP2Toggled(bool val);
    void slotBtnP3Toggled(bool val);
    void slotBtnP4Toggled(bool val);
    void slotBtnP5Toggled(bool val);
    void slotBtnP6Toggled(bool val);
    void slotBtnP7Toggled(bool val);

 protected:
    bool event(QEvent *e) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void paintEvent(QPaintEvent *event_) override;

 signals:
    void signalHandleResponse();
    void signalBtn1Toggled(bool val);
    void signalBtn2Toggled(bool val);
    void signalBtn3Toggled(bool val);
    void signalBtn4Toggled(bool val);
    void signalBtn5Toggled(bool val);
    void signalBtn6Toggled(bool val);
    void signalBtn7Toggled(bool val);
    void signalBtn8Toggled(bool val);
    void signalBtn9Toggled(bool val);
    void signalBtn0Toggled(bool val);
    void signalBtnP1Toggled(bool val);
    void signalBtnP2Toggled(bool val);
    void signalBtnP3Toggled(bool val);
    void signalBtnP4Toggled(bool val);
    void signalBtnP5Toggled(bool val);
    void signalBtnP6Toggled(bool val);
    void signalBtnP7Toggled(bool val);

 private:
    AttributeType response_;
    AttributeType ledResponse_;
    IGui *igui_;

    QPixmap pixmapBkg_;
    QPixmap pixmapBtn_[KEY_Total];
    QPoint pointCursor_;
    QRect rectBtn_[KEY_Total];

    int btnSelected_;
    int btnPressed_;
};

}  // namespace debugger
