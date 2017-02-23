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

    paletteDefault_.setColor(QPalette::Text, Qt::black);
    paletteDefault_.setColor(QPalette::Base, Qt::white);


    QGridLayout *gridLayout = new QGridLayout(this);
    setLayout(gridLayout);


    QLabel *lbl = new QLabel("Filter:");
    gridLayout->addWidget(lbl, 0, 0, Qt::AlignRight);

    editFilter_ = new QLineEdit(this);
    editFilter_->setText(tr("*"));
    editFilter_->setFixedWidth(fm.width(tr("*some_test_func*")));
    editFilter_->setPalette(paletteDefault_);
    gridLayout->addWidget(editFilter_, 0, 1, Qt::AlignLeft);
    gridLayout->setColumnStretch(1, 10);

    connect(editFilter_, SIGNAL(returnPressed()),
            this, SLOT(slotFilterEditingFinished()));
}

void SymbolBrowserControl::slotFilterEditingFinished() {
    emit signalFilterChanged(editFilter_->text());
}



}  // namespace debugger
