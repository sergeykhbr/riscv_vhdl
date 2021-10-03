/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Editor control panel.
 */

#include "MemControl.h"
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace debugger {

MemControl::MemControl(QWidget *parent, uint64_t addr, uint64_t sz) 
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


    QLabel *lbl = new QLabel("Addr,HEX:");
    gridLayout->addWidget(lbl, 0, 0, Qt::AlignRight);

    editAddr_ = new QLineEdit(this);
    editAddr_->setText(QString("%1").arg(addr, 16, 16, QChar('0')));
    editAddr_->setFixedWidth(8 + fm.horizontalAdvance(editAddr_->text()));
    editAddr_->setPalette(paletteDefault_);
    gridLayout->addWidget(editAddr_, 0, 1, Qt::AlignLeft);

    QLabel *lbl2 = new QLabel("Size,B:");
    gridLayout->addWidget(lbl2, 0, 2, Qt::AlignRight);

    editBytes_ = new QLineEdit(this);
    editBytes_->setText(QString("%1").arg(sz, 0, 10));
    editBytes_->setFixedWidth(8 + fm.horizontalAdvance("000000"));
    editBytes_->setPalette(paletteDefault_);
    gridLayout->addWidget(editBytes_, 0, 3, Qt::AlignLeft);

    QPushButton *btnUpdate = new QPushButton(tr("&Update"));
    btnUpdate->setFlat(false);
    btnUpdate->setCheckable(false);
    gridLayout->addWidget(btnUpdate, 0, 4, Qt::AlignLeft);
    gridLayout->setColumnStretch(4, 10);

    connect(editAddr_, SIGNAL(textChanged(const QString &)),
                       this, SLOT(slotModified()));
    connect(editBytes_, SIGNAL(textChanged(const QString &)),
                        this, SLOT(slotModified()));
    connect(btnUpdate, SIGNAL(released()), this, SLOT(slotUpdate()));

    cmd_.make_list(2);
}

void MemControl::slotModified() {
    editAddr_->setPalette(paletteModified_);
    editBytes_->setPalette(paletteModified_);
}

void MemControl::slotUpdate() {
    editAddr_->setPalette(paletteDefault_);
    editBytes_->setPalette(paletteDefault_);

    if (!isChanged()) {
        return;
    }
    uint64_t adr = editAddr_->text().toLongLong(0, 16);
    uint64_t sz = editBytes_->text().toLongLong(0, 10);
    cmd_[0u].make_uint64(adr);
    cmd_[1].make_uint64(sz);

    emit signalAddressChanged(&cmd_);
}

bool MemControl::isChanged() {
    return true;
}


}  // namespace debugger
