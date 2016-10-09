#include "coreservices/isocinfo.h"
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
    igui_->registerWidgetInterface(static_cast<ISignalListener *>(this));
    igpio_ = 0;
    //needUpdate_ = false;
    //leds_ = 0;
    //dips_ = 0;

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

GpioWidget::~GpioWidget() {
    if (igpio_) {
        igpio_->unregisterSignalListener(static_cast<ISignalListener *>(this));
    }
}

void GpioWidget::closeEvent(QCloseEvent *event_) {
    AttributeType tmp;
    emit signalClose(this, tmp);
    event_->accept();
}

void GpioWidget::updateSignal(int start, int width, uint64_t value) {
    uint64_t mask = (1 << (start + width)) - 1;
    mask = (mask >> start) << start;
    //leds_ &= ~mask;
    //leds_ |= (value & mask);
    //needUpdate_ = true;
}

void GpioWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    GpioType gpio;
    gpio.u.val[0] = resp->to_uint64();
    emit signalLedValue(gpio.u.map.led);
    emit signalDipValue(gpio.u.map.dip);
}

void GpioWidget::slotPostInit(AttributeType *cfg) {
    char tstr[64];
    ISocInfo *info = static_cast<ISocInfo *>(igui_->getSocInfo());
    uint32_t addr_gpio = static_cast<int>(info->addressGpio());
    RISCV_sprintf(tstr, sizeof(tstr), "read %08x 8", addr_gpio);
    cmdRd_.make_string(tstr);
}

void GpioWidget::slotClosingMainForm() {
    igui_->unregisterWidgetInterface(static_cast<ISignalListener *>(this));
}

void GpioWidget::slotUpdateByTimer() {
    if (!isVisible()) {
        return;
    }
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmdRd_, true);
}

}  // namespace debugger
