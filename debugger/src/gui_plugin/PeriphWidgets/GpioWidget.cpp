#include "GpioWidget.h"
#include "moc_GpioWidget.h"

#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <memory>

namespace debugger {

GpioWidget::GpioWidget(IGui *igui, QWidget *parent) : QWidget(parent) {
    igui_ = igui;
    igui_->registerWidgetInterface(static_cast<ISignalListener *>(this));
    igpio_ = 0;
    needUpdate_ = false;
    leds_ = 0;
    dips_ = 0;

    setWindowTitle(tr("gpio0"));
    setMinimumWidth(150);
    setMinimumHeight(100);
    QImage img(tr(":/images/toggle.png"));
    pixmapBkg_ = QPixmap(size()).fromImage(img);
    pixmapGreen_ = QPixmap(QSize(6, 10));
    pixmapGreen_ .fill(QColor("#10A010"));
    pixmapGrey_ = QPixmap(QSize(6, 10));
    pixmapGrey_ .fill(QColor("#101010"));

    pollingTimer_ = new QTimer(this);
    pollingTimer_->setSingleShot(false);
    connect(pollingTimer_, SIGNAL(timeout()), this, SLOT(slotPollingUpdate()));
}

GpioWidget::~GpioWidget() {
    if (igpio_) {
        igpio_->unregisterSignalListener(static_cast<ISignalListener *>(this));
    }
}

void GpioWidget::resizeEvent(QResizeEvent *ev) {
    int w = ev->size().width();
    int h = ev->size().height();
    if (h == 0 || w == 0)  {
        return;
    }
}

void GpioWidget::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.drawPixmap(QPoint(0, 0), pixmapBkg_);
    for (int i = 0; i < 8; i++) {
        if (leds_ & (1ull << i)) {
            p.drawPixmap(QPoint(10 + 14*i, 20), pixmapGreen_);
        } else {
            p.drawPixmap(QPoint(10 + 14*i, 20), pixmapGrey_);
        }
    }
    p.end();
}

void GpioWidget::closeEvent(QCloseEvent *event_) {
    AttributeType tmp;
    emit signalClose(this, tmp);
    event_->accept();
}

void GpioWidget::updateSignal(int start, int width, uint64_t value) {
    uint64_t mask = (1 << (start + width)) - 1;
    mask = (mask >> start) << start;
    leds_ &= ~mask;
    leds_ |= (value & mask);
    needUpdate_ = true;
}

void GpioWidget::handleResponse(AttributeType *req, AttributeType *resp) {
}

void GpioWidget::slotConfigure(AttributeType *cfg) {
    for (unsigned i = 0; i < cfg->size(); i++) {
        const AttributeType &attr = (*cfg)[i];
        if (strcmp(attr[0u].to_string(), "Gpio") == 0) {
            igpio_ = static_cast<ISignal *>
                (RISCV_get_service_iface(attr[1].to_string(), IFACE_SIGNAL));
            if (igpio_) {
                igpio_->registerSignalListener(
                        static_cast<ISignalListener *>(this));
            }
        } else if (strcmp(attr[0u].to_string(), "PollingMs") == 0) {
            int ms = static_cast<int>(attr[1].to_uint64());
            if (ms != 0) {
                pollingTimer_->start(ms);
            }
        }
    }
}

void GpioWidget::slotRepaintByTimer() {
    if (!needUpdate_) {
        return;
    }
    needUpdate_ = false;
    update();
}

void GpioWidget::slotClosingMainForm() {
    igui_->unregisterWidgetInterface(static_cast<ISignalListener *>(this));
}

static int tst_led = 0x4;
void GpioWidget::slotPollingUpdate() {
    AttributeType cmd_tmp2, led_value;
    cmd_tmp2.from_config("['write',0x80000000,4]");
    led_value.make_uint64(tst_led++);
    cmd_tmp2.add_to_list(&led_value);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmd_tmp2);
}

}  // namespace debugger
