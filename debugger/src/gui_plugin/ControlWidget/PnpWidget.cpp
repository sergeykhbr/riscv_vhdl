#include "coreservices/isocinfo.h"
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
    reqCnt_ = 0;
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
    switch (reqCnt_) {
    case 0: // target info
        {
            QImage img(tr(":/images/ml605_top.png"));
            pixmapBkg_ = QPixmap(size()).fromImage(img);
            labelBoard_->setPixmap(pixmapBkg_);

            update();
        }
        break;
    default:;
    }
    reqCnt_++;
}

void PnpWidget::slotConfigDone() {
    AttributeType cmd;
    char tstr[64];

    ISocInfo *info = static_cast<ISocInfo *>(igui_->getSocInfo());
    uint32_t addr_pnp = static_cast<int>(info->addressPlugAndPlay());
    RISCV_sprintf(tstr, sizeof(tstr), "read 0x%08x 4", addr_pnp);

    cmd.make_string(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmd, true);
}


}  // namespace debugger
