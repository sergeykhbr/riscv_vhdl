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

#include "DemoSTM32Keypad.h"
#include <memory>
#include <string.h>

namespace debugger {

static const int BTN_DIGIT_WIDTH = 37;
static const int BTN_DIGIT_HEIGHT = 32;

typedef void (DemoSTM32Keypad::*toggle_signal_type)(bool);

static const char *strcmd[2*DemoSTM32Keypad::KEY_Total] = {
    "BTN_1 release",
    "BTN_1 press",
    "BTN_2 release",
    "BTN_2 press",
    "BTN_3 release",
    "BTN_3 press",
    "BTN_4 release",
    "BTN_4 press",
    "BTN_5 release",
    "BTN_5 press",
    "BTN_6 release",
    "BTN_6 press",
    "BTN_7 release",
    "BTN_7 press",
    "BTN_8 release",
    "BTN_8 press",
    "BTN_9 release",
    "BTN_9 press",
    "BTN_0 release",
    "BTN_0 press",
    "BTN_P1 release",
    "BTN_P1 press",
    "BTN_P2 release",
    "BTN_P2 press",
    "BTN_P3 release",
    "BTN_P3 press",
    "BTN_P4 release",
    "BTN_P4 press",
    "BTN_P5 release",
    "BTN_P5 press",
    "BTN_P6 release",
    "BTN_P6 press",
    "BTN_P7 release",
    "BTN_P7 press",
};

DemoSTM32Keypad::DemoSTM32Keypad(QWidget *parent, IGui *igui)
    : QWidget(parent) {
    igui_ = igui;
    setFixedWidth(320);
    setFixedHeight(120);

    QWidget *qgui = reinterpret_cast<QWidget *>(igui_->getQGui());
    connect(qgui, SIGNAL(signalExternalCommand(const char *)),
            this, SLOT(slotExternalCommand(const char *)));

    connect(this, SIGNAL(signalHandleResponse()), SLOT(slotHandleResponse()));
    connect(this, SIGNAL(signalBtn1Toggled(bool)),
                  SLOT(slotBtn1Toggled(bool)));
    connect(this, SIGNAL(signalBtn2Toggled(bool)),
                  SLOT(slotBtn2Toggled(bool)));
    connect(this, SIGNAL(signalBtn3Toggled(bool)),
                  SLOT(slotBtn3Toggled(bool)));
    connect(this, SIGNAL(signalBtn4Toggled(bool)),
                  SLOT(slotBtn4Toggled(bool)));
    connect(this, SIGNAL(signalBtn5Toggled(bool)),
                  SLOT(slotBtn5Toggled(bool)));
    connect(this, SIGNAL(signalBtn6Toggled(bool)),
                  SLOT(slotBtn6Toggled(bool)));
    connect(this, SIGNAL(signalBtn7Toggled(bool)),
                  SLOT(slotBtn7Toggled(bool)));
    connect(this, SIGNAL(signalBtn8Toggled(bool)),
                  SLOT(slotBtn8Toggled(bool)));
    connect(this, SIGNAL(signalBtn9Toggled(bool)),
                  SLOT(slotBtn9Toggled(bool)));
    connect(this, SIGNAL(signalBtn0Toggled(bool)),
                  SLOT(slotBtn0Toggled(bool)));
    connect(this, SIGNAL(signalBtnP1Toggled(bool)),
                  SLOT(slotBtnP1Toggled(bool)));
    connect(this, SIGNAL(signalBtnP2Toggled(bool)),
                  SLOT(slotBtnP2Toggled(bool)));
    connect(this, SIGNAL(signalBtnP3Toggled(bool)),
                  SLOT(slotBtnP3Toggled(bool)));
    connect(this, SIGNAL(signalBtnP4Toggled(bool)),
                  SLOT(slotBtnP4Toggled(bool)));
    connect(this, SIGNAL(signalBtnP5Toggled(bool)),
                  SLOT(slotBtnP5Toggled(bool)));
    connect(this, SIGNAL(signalBtnP6Toggled(bool)),
                  SLOT(slotBtnP6Toggled(bool)));
    connect(this, SIGNAL(signalBtnP7Toggled(bool)),
                  SLOT(slotBtnP7Toggled(bool)));
    //igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
    //                       objname_.to_string(), &response_, true);

    pixmapBkg_ = QPixmap(size());
    pixmapBkg_.fill(Qt::white);
    pixmapBtn_[KEY_1].convertFromImage(
        QImage(":/images/demo_stm32/1_37x32x4.png"));
    pixmapBtn_[KEY_2].convertFromImage(
        QImage(":/images/demo_stm32/2_37x32x4.png"));
    pixmapBtn_[KEY_3].convertFromImage(
        QImage(":/images/demo_stm32/3_37x32x4.png"));
    pixmapBtn_[KEY_4].convertFromImage(
        QImage(":/images/demo_stm32/4_37x32x4.png"));
    pixmapBtn_[KEY_5].convertFromImage(
        QImage(":/images/demo_stm32/5_37x32x4.png"));
    pixmapBtn_[KEY_6].convertFromImage(
        QImage(":/images/demo_stm32/6_37x32x4.png"));
    pixmapBtn_[KEY_7].convertFromImage(
        QImage(":/images/demo_stm32/7_37x32x4.png"));
    pixmapBtn_[KEY_8].convertFromImage(
        QImage(":/images/demo_stm32/8_37x32x4.png"));
    pixmapBtn_[KEY_9].convertFromImage(
        QImage(":/images/demo_stm32/9_37x32x4.png"));
    pixmapBtn_[KEY_0].convertFromImage(
        QImage(":/images/demo_stm32/0_37x32x4.png"));
    pixmapBtn_[KEY_P1].convertFromImage(
        QImage(":/images/demo_stm32/p1_37x32x4.png"));
    pixmapBtn_[KEY_P2].convertFromImage(
        QImage(":/images/demo_stm32/p2_37x32x4.png"));
    pixmapBtn_[KEY_P3].convertFromImage(
        QImage(":/images/demo_stm32/p3_37x32x4.png"));
    pixmapBtn_[KEY_P4].convertFromImage(
        QImage(":/images/demo_stm32/p4_37x32x4.png"));
    pixmapBtn_[KEY_P5].convertFromImage(
        QImage(":/images/demo_stm32/p5_37x32x4.png"));
    pixmapBtn_[KEY_P6].convertFromImage(
        QImage(":/images/demo_stm32/p6_37x32x4.png"));
    pixmapBtn_[KEY_P7].convertFromImage(
        QImage(":/images/demo_stm32/p7_37x32x4.png"));

    rectBtn_[KEY_1] = QRect(40, 15, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_2] = QRect(80, 15, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_3] = QRect(120, 15, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_4] = QRect(160, 15, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_5] = QRect(200, 15, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_6] = QRect(40, 50, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_7] = QRect(80, 50, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_8] = QRect(120, 50, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_9] = QRect(160, 50, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_0] = QRect(200, 50, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_P1] = QRect(240, 15, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_P3] = QRect(240, 50, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);

    rectBtn_[KEY_P7] = QRect(40, 85, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_P4] = QRect(80, 85, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_P5] = QRect(160, 85, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_P6] = QRect(200, 85, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);
    rectBtn_[KEY_P2] = QRect(240, 85, BTN_DIGIT_WIDTH, BTN_DIGIT_HEIGHT);

 
    setMouseTracking(true);
    btnSelected_ = 0;
    btnPressed_ = 0;
}

DemoSTM32Keypad::~DemoSTM32Keypad() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void DemoSTM32Keypad::slotUpdateByTimer() {
}

void DemoSTM32Keypad::handleResponse(const char *cmd) {
    static const char *keys[KEY_Total] = {
        "BTN_1", "BTN_2", "BTN_3",
        "BTN_4", "BTN_5", "BTN_6",
        "BTN_7", "BTN_8", "BTN_9",
        "BTN_0", "BTN_P1", "BTN_P2",
        "BTN_P3", "BTN_P4", "BTN_P5",
        "BTN_P6", "BTN_P7"
    };
    for (int i = 0; i < KEY_Total; i++) {
        if (strstr(cmd, keys[i]) != 0) {
            if (response_.to_bool()) {
                btnPressed_ |= (1 << i);
            } else {
                btnPressed_ &= ~(1 << i);
            }
            break;
        }
    }
    emit signalHandleResponse();
}

void DemoSTM32Keypad::slotHandleResponse() {
    update();
}

void DemoSTM32Keypad::slotBtn1Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_1 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn2Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_2 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn3Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_3 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn4Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_4 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn5Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_5 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn6Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_6 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn7Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_7 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn8Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_8 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn9Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_9 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtn0Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_0 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtnP1Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_P1 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtnP2Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_P2 + val], &response_, true);
}
void DemoSTM32Keypad::slotBtnP3Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_P3 + val], &response_, true);
}
void DemoSTM32Keypad::slotBtnP4Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_P4 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtnP5Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_P5 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtnP6Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_P6 + val], &response_, true);
}

void DemoSTM32Keypad::slotBtnP7Toggled(bool val) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            strcmd[2*KEY_P7 + val], &response_, true);
}

void DemoSTM32Keypad::slotExternalCommand(const char *cmd) {
    for (int i = 0; i < 2*KEY_Total; i++) {
        if (strcmp(cmd, strcmd[i]) == 0) {
            if (i & 0x1) {
                btnPressed_ |= (1 << (i >> 1));
            } else {
                btnPressed_ &= ~(1 << (i >> 1));
            }
        }
    }
    emit signalHandleResponse();
}

bool DemoSTM32Keypad::event(QEvent *e) {
    if (e->type() == QEvent::Leave){
        btnSelected_ = 0;
        update();
    }
    return QWidget::event(e);
}

void DemoSTM32Keypad::mouseMoveEvent(QMouseEvent *event) {
    pointCursor_ = event->pos();

    int sel = 0;
    for (int i = 0; i < KEY_Total; i++) {
        if (rectBtn_[i].contains(pointCursor_)) {
            sel |= 1 << i;
        }
    }
    if (sel != btnSelected_) {
        btnSelected_ = sel;
        update();
    }
}

void DemoSTM32Keypad::mousePressEvent(QMouseEvent *event) {
    setFocus();
    /** There're 2 methods: buttons() and button():
        buttons() is a multiple flag of button()
    */
    Qt::MouseButton pressed = event->button();
    if (pressed != Qt::LeftButton) {
        pressed = Qt::NoButton;
        return;
    }

    int press = btnPressed_;
    toggle_signal_type s[KEY_Total] = {
        &DemoSTM32Keypad::signalBtn1Toggled,
        &DemoSTM32Keypad::signalBtn2Toggled,
        &DemoSTM32Keypad::signalBtn3Toggled,
        &DemoSTM32Keypad::signalBtn4Toggled,
        &DemoSTM32Keypad::signalBtn5Toggled,
        &DemoSTM32Keypad::signalBtn6Toggled,
        &DemoSTM32Keypad::signalBtn7Toggled,
        &DemoSTM32Keypad::signalBtn8Toggled,
        &DemoSTM32Keypad::signalBtn9Toggled,
        &DemoSTM32Keypad::signalBtn0Toggled,
        &DemoSTM32Keypad::signalBtnP1Toggled,
        &DemoSTM32Keypad::signalBtnP2Toggled,
        &DemoSTM32Keypad::signalBtnP3Toggled,
        &DemoSTM32Keypad::signalBtnP4Toggled,
        &DemoSTM32Keypad::signalBtnP5Toggled,
        &DemoSTM32Keypad::signalBtnP6Toggled,
        &DemoSTM32Keypad::signalBtnP7Toggled,
    };

    for (int n = 0; n < KEY_Total; n++) {
        if (rectBtn_[n].contains(pointCursor_)) {
            press ^= 1 << n;
            bool toggle = press & (1 << n) ? true : false;
            emit (this->*(s[n]))(toggle);
        }
    }
    if (press != btnPressed_) {
        btnPressed_ = press;
        update();
    }
}

void DemoSTM32Keypad::paintEvent(QPaintEvent *event) {
    QPixmap pixmapPaint(pixmapBkg_.size());
    
    QPainter p1(&pixmapPaint);

    p1.drawPixmap(QPoint(0, 0), pixmapBkg_);

    int press;
    int sel;
    for (int i = 0; i < KEY_Total; i++) {
        press = (btnPressed_ >> i) & 0x1;
        sel = (btnSelected_ >> i) & 0x1;
        int pheight = pixmapBtn_[i].height();
        int pwidth = pixmapBtn_[i].width()/4;
        if (press && sel) {
            p1.drawPixmap(rectBtn_[i], pixmapBtn_[i].copy(
                        3*pwidth, 0, pwidth, pheight));
        } else if (press && !sel) {
            p1.drawPixmap(rectBtn_[i], pixmapBtn_[i].copy(
                        2*pwidth, 0, pwidth, pheight));
        } else if (!press && sel) {
            p1.drawPixmap(rectBtn_[i], pixmapBtn_[i].copy(
                        1*pwidth, 0, pwidth, pheight));
        } else {
            p1.drawPixmap(rectBtn_[i], pixmapBtn_[i].copy(
                        0, 0, pwidth, pheight));
        }
    }
    p1.end();

    QPainter p(this);
    p.drawPixmap(QPoint(0,0), pixmapPaint);
    p.end();
}

}  // namespace debugger
