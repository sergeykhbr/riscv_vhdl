/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      MDI container of all other widgets.
 */
#pragma once

#include "attribute.h"

#include <QtWidgets/QtWidgets>
#include <QtWidgets/QMdiArea>

namespace debugger {

class MdiAreaWidget : public QMdiArea
{
    Q_OBJECT

public:
    MdiAreaWidget(AttributeType &cfg, QWidget *parent = 0);

private slots:
    void slotRemoveView(AttributeType &cfg);

private:
    QMdiSubWindow *findMdiChild(const char* name);

    AttributeType Config_;

};

}  // namespace debugger
