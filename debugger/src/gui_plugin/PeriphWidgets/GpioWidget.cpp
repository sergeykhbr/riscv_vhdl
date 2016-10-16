#include "LedArea.h"
#include "DipArea.h"
#include "GpioWidget.h"
#include "moc_GpioWidget.h"

#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <memory>

namespace debugger {

GpioWidget::GpioWidget(IGui *igui, QWidget *parent)
    : UnclosableWidget(parent) {
    igui_ = igui;
    newValue_.u.val[0] = 0;

    //setMinimumWidth(150);
    //setMinimumHeight(100); 


    QGridLayout *layout = new QGridLayout(this);

    // Row 0:
    QLabel *lbl1 = new QLabel(this);
    lbl1->setText(tr("User defined LEDs"));
    layout->addWidget(lbl1, 0, 0, 1, 2, Qt::AlignCenter);

    // Row 1
    LedArea *ledArea = new LedArea(this);
    layout->addWidget(ledArea, 1, 0, Qt::AlignCenter);
    connect(this, SIGNAL(signalLedValue(uint32_t)),
            ledArea, SLOT(slotUpdate(uint32_t)));

    QLabel *lbl2 = new QLabel(this);
    lbl2->setText(tr("xx"));
    layout->addWidget(lbl2, 1, 1, Qt::AlignLeft);

    // Row 2:
    QLabel *lbl3 = new QLabel(this);
    lbl3->setText(tr("User defined DIPs"));
    layout->addWidget(lbl3, 2, 0, 1, 2, Qt::AlignCenter);

    // Row 3:
    DipArea *dipArea = new DipArea(this);
    layout->addWidget(dipArea, 3, 0, Qt::AlignCenter);
    connect(this, SIGNAL(signalDipValue(uint32_t)),
            dipArea, SLOT(slotUpdate(uint32_t)));

    QLabel *lbl4 = new QLabel(this);
    lbl4->setText(tr("xx"));
    layout->addWidget(lbl4, 3, 1, Qt::AlignLeft);

    setLayout(layout);
}

void GpioWidget::closeEvent(QCloseEvent *event_) {
    AttributeType tmp;
    emit signalClose(this, tmp);
    event_->accept();
}

void GpioWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    newValue_.u.val[0] = resp->to_uint64();
}

void GpioWidget::slotPostInit(AttributeType *cfg) {
    char tstr[64];
    ISocInfo *info = static_cast<ISocInfo *>(igui_->getSocInfo());
    uint32_t addr_gpio = static_cast<int>(info->addressGpio());
    RISCV_sprintf(tstr, sizeof(tstr), "read 0x%08x 8", addr_gpio);
    cmdRd_.make_string(tstr);
}

void GpioWidget::slotUpdateByTimer() {
    if (!isVisible()) {
        return;
    }
    value_ = newValue_;
    emit signalLedValue(value_.u.map.led);
    emit signalDipValue(value_.u.map.dip);

    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmdRd_, true);
}

}  // namespace debugger
