#include "PnpWidget.h"
#include "moc_PnpWidget.h"

#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <memory>

namespace debugger {

enum EPnpAddr {
    PNP_HW_ID = 0xffff0000,
};

PnpWidget::PnpWidget(IGui *igui, QWidget *parent) : UnclosableWidget(parent) {
    igui_ = igui;

    setWindowTitle(tr("pnp0"));

    mainLayout_ = new QGridLayout(this);

    setMinimumWidth(150);
    setMinimumHeight(100);
    labelBoard_ = new QLabel(this);
    labelBoard_->setScaledContents(false);
}

PnpWidget::~PnpWidget() {
}

void PnpWidget::paintEvent(QPaintEvent *event) {
    QLabel *labelTop = new QLabel(tr("Target:"));
    mainLayout_->setColumnStretch(1, 10);
    mainLayout_->setRowStretch(1, 10);
    
    mainLayout_->addWidget(labelTop, 0, 0, Qt::AlignCenter);
    mainLayout_->addWidget(labelBoard_, 1, 0, Qt::AlignVCenter);
}

void PnpWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    QImage img(tr(":/images/ml605_top.png"));
    pixmapBkg_ = QPixmap(size()).fromImage(img);
    labelBoard_->setPixmap(pixmapBkg_);

    update();
}

void PnpWidget::slotConfigure(AttributeType *cfg) {
    AttributeType cmd_rd;
    cmd_rd.from_config("['read',0x80000000,4]");
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmd_rd);
}

}  // namespace debugger
