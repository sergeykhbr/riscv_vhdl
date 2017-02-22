/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Symbol Browser control panel.
 */

#include "SymbolBrowserControl.h"
#include "moc_SymbolBrowserControl.h"

#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace debugger {

SymbolBrowserControl::SymbolBrowserControl(QWidget *parent) 
    : QWidget(parent) {
    QFont font = QFont("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    QFontMetrics fm(font);

    paletteModified_.setColor(QPalette::Base, Qt::yellow);
    paletteModified_.setColor(QPalette::Text, Qt::black);

    paletteDefault_.setColor(QPalette::Text, Qt::black);
    paletteDefault_.setColor(QPalette::Base, Qt::white);


    QGridLayout *gridLayout = new QGridLayout(this);
    setLayout(gridLayout);


    QLabel *lbl = new QLabel("Source view:");
    gridLayout->addWidget(lbl, 0, 0, Qt::AlignRight);


    cmd_.make_list(2);
}

void SymbolBrowserControl::slotModified() {
}

void SymbolBrowserControl::slotUpdate() {
    if (!isChanged()) {
        return;
    }
}

bool SymbolBrowserControl::isChanged() {
    return true;
}


}  // namespace debugger
