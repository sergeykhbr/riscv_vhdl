/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Symbol Browser control panel.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>


namespace debugger {

class SymbolBrowserControl : public QWidget {
    Q_OBJECT
public:
    explicit SymbolBrowserControl(QWidget *parent);

signals:
    void signalFilterChanged(const QString &flt);

public slots:
    void slotFilterEditingFinished();

private:
    QLineEdit *editFilter_;
    QPalette paletteDefault_;
};

}  // namespace debugger
