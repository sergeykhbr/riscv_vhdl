#include "LedArea.h"
#include "GpioWidget.h"
#include "moc_GpioWidget.h"

#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <memory>

namespace debugger {

GpioWidget::GpioWidget(IGui *igui, QWidget *parent) : QWidget(parent) {
    igui_ = igui;
    newValue_.u.val[0] = 0;

    setWindowTitle(tr("gpio0"));
    setMinimumWidth(150);
    setMinimumHeight(100);

    LedArea *ledArea = new LedArea(this);
    connect(this, SIGNAL(signalLedValue(uint32_t)),
            ledArea, SLOT(slotUpdate(uint32_t)));

    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);
    layout->addWidget(ledArea, 1, 1, Qt::AlignCenter);

    QLabel *lbl1 = new QLabel(this);
    lbl1->setText(tr("User defined LEDs"));
    layout->addWidget(ledArea, 1, 2, Qt::AlignCenter);
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
    if (newValue_.u.map.led != value_.u.map.led) {
        value_.u.map.led = newValue_.u.map.led;
        emit signalLedValue(value_.u.map.led);
    }
    if (newValue_.u.map.dip != value_.u.map.dip) {
        value_.u.map.dip = newValue_.u.map.dip;
        emit signalDipValue(value_.u.map.dip);
    }

    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmdRd_, true);
}

}  // namespace debugger
