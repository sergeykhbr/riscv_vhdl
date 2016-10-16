/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      DIP's area renderer declaration.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtGui/QPaintEvent>

namespace debugger {

class DipArea : public QWidget {
    Q_OBJECT
public:
    DipArea(QWidget *parent = 0);

private slots:
    void slotUpdate(uint32_t val);

protected:
    void paintEvent(QPaintEvent *event_) Q_DECL_OVERRIDE;

private:
    AttributeType dipTotal_;
    uint32_t dips_;
    QPixmap pixmapBody_;
    QPixmap pixmapDown_;
};

}  // namespace debugger
