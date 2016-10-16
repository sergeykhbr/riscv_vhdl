#include "coreservices/isocinfo.h"
#include "PnpWidget.h"
#include "moc_PnpWidget.h"

#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <memory>

namespace debugger {

enum EWName {
    WTarget_Name,
    WTarget_image,
    WHW_ID,
    WFW_ID,
    WMasterTotal,
    WSlaveTotal,
    W_Total
};

PnpWidget::PnpWidget(IGui *igui, QWidget *parent) : UnclosableWidget(parent) {
    igui_ = igui;

    mainLayout_ = new QGridLayout(this);

    setMinimumWidth(150);
    setMinimumHeight(100);

    imgTarget_[SIMULATION] = QImage(tr(":/images/ml605_top.png"));
    imgTarget_[ML605] = QImage(tr(":/images/ml605_top.png"));
    imgTarget_[KC705] = QImage(tr(":/images/ml605_top.png"));
    imgTarget_[TARGET_Unknown] = QImage(tr(":/images/ml605_top.png"));


    QLabel *labelBoardImg = new QLabel(this);
    labelBoardImg->setScaledContents(false);
    
    // row, column, rowSpan, colSpan:
    mainLayout_->addWidget(new QLabel(tr("Target: --")), 0, 0, 1, 2, Qt::AlignCenter);
    // row span 5: hwid, swid, slvtotal, msttotal, empty_row, empty_row
    mainLayout_->addWidget(labelBoardImg, 1, 0, 6, 1, Qt::AlignVCenter);
    mainLayout_->addWidget(new QLabel(tr("HW_ID:")), 1, 1, Qt::AlignLeft);
    mainLayout_->addWidget(new QLabel(tr("FW_ID:")), 2, 1, Qt::AlignLeft);
    mainLayout_->addWidget(new QLabel(tr("Masters Total:")), 3, 1, Qt::AlignLeft);
    mainLayout_->addWidget(new QLabel(tr("Slave Total:")), 4, 1, Qt::AlignLeft);

    mainLayout_->setColumnStretch(1, 10);

    connect(this, SIGNAL(signalUpdate()), this, SLOT(slotUpdate()));
    setMinimumWidth(400);
    setMinimumHeight(300);
}

PnpWidget::~PnpWidget() {
}

void PnpWidget::showEvent(QShowEvent *event_) {
    AttributeType cmd;
    char tstr[64];

    ISocInfo *info = static_cast<ISocInfo *>(igui_->getSocInfo());
    uint32_t addr_pnp = static_cast<int>(info->addressPlugAndPlay());
    RISCV_sprintf(tstr, sizeof(tstr), "read 0x%08x 256", addr_pnp);

    cmd.make_string(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &cmd, true);
    
    UnclosableWidget::showEvent(event_);
}

void PnpWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    if (!resp->is_data()) {
        return;
    }
    memcpy(&pnp_, resp->data(), resp->size());
    emit signalUpdate();
}

void PnpWidget::slotUpdate() {
    char tstr[256];
    QPixmap pixmapBkg;
    QString targetText;
    switch (pnp_.tech.bits.tech) {
    case TECH_INFERRED:
        targetText = QString("Target: Simulation");
        pixmapBkg = QPixmap(size()).fromImage(imgTarget_[SIMULATION]);
        break;
    case TECH_VIRTEX6:
        targetText = QString("Target: ML605");
        pixmapBkg = QPixmap(size()).fromImage(imgTarget_[ML605]);
        break;
    case TECH_KINTEX7:
        targetText = QString("Target: KC705");
        pixmapBkg = QPixmap(size()).fromImage(imgTarget_[KC705]);
        break;
    default:
        targetText = QString("Target: --");
        pixmapBkg = QPixmap(size()).fromImage(imgTarget_[TARGET_Unknown]);
    }
    getLabel(WTarget_Name)->setText(targetText);
    getLabel(WTarget_image)->setPixmap(pixmapBkg);

    QFontMetrics fm(font());
    int h = pixmapBkg.size().height() + fm.height() + 50;
    int w = pixmapBkg.size().width();

    RISCV_sprintf(tstr, sizeof(tstr), "HW_ID: 0x%08x", pnp_.hwid);
    getLabel(WHW_ID)->setText(QString(tstr));
    w += fm.width(QString(tstr)) + 40;

    RISCV_sprintf(tstr, sizeof(tstr), "FW_ID: 0x%08x", pnp_.fwid);
    getLabel(WFW_ID)->setText(QString(tstr));
    
    RISCV_sprintf(tstr, sizeof(tstr), "MST Total: %d", pnp_.tech.bits.mst_total);
    getLabel(WMasterTotal)->setText(QString(tstr));

    RISCV_sprintf(tstr, sizeof(tstr), "SLV Total: %d", pnp_.tech.bits.slv_total);
    getLabel(WSlaveTotal)->setText(QString(tstr));

    /**
     * Lines with index 5 and 6 are empty.
     */

    uint32_t adr1, adr2;
    for (uint8_t i = 0; i < pnp_.tech.bits.slv_total; i++) {
        adr1 = pnp_.slaves[i].xaddr;
        adr2 = adr1 + ~pnp_.slaves[i].xmask;
        RISCV_sprintf(tstr, sizeof(tstr), "0x%08x .. 0x%08x", adr1, adr2);
        if ((W_Total + 2*i) < mainLayout_->count()) {
            getLabel(W_Total + 2*i)->setText(QString(tstr));
        } else {
            mainLayout_->addWidget(new QLabel(tstr), 7 + 2*i, 0, Qt::AlignLeft);
        }

        switch (pnp_.slaves[i].did) {
        case GNSSSENSOR_DUMMY:
            RISCV_sprintf(tstr, sizeof(tstr), "Dummy/Empty slot", NULL);
            break;
        case GNSSSENSOR_BOOTROM:
            RISCV_sprintf(tstr, sizeof(tstr), "Boot ROM", NULL);
            break;
        case GNSSSENSOR_FWIMAGE:
            RISCV_sprintf(tstr, sizeof(tstr), "FwImage ROM", NULL);
            break;
        case GNSSSENSOR_SRAM:
            RISCV_sprintf(tstr, sizeof(tstr), "SRAM", NULL);
            break;
        case GNSSSENSOR_IRQCTRL:
            RISCV_sprintf(tstr, sizeof(tstr), "Interrupt Controller", NULL);
            break;
        case GNSSSENSOR_UART:
            RISCV_sprintf(tstr, sizeof(tstr), "UART", NULL);
            break;
        case GNSSSENSOR_GPIO:
            RISCV_sprintf(tstr, sizeof(tstr), "GPIO/LEDs", NULL);
            break;
        case GNSSSENSOR_ENGINE_STUB:
            RISCV_sprintf(tstr, sizeof(tstr), "GNSS Enginge stub", NULL);
            break;
        case GNSSSENSOR_ETHMAC:
            RISCV_sprintf(tstr, sizeof(tstr), "Ethernet MAC 10/100", NULL);
            break;
        case GNSSSENSOR_GPTIMERS:
            RISCV_sprintf(tstr, sizeof(tstr), "GP Timers", NULL);
            break;
        case GNSSSENSOR_PNP:
            RISCV_sprintf(tstr, sizeof(tstr), "Plug'n'Play support", NULL);
            break;
        default:
            RISCV_sprintf(tstr, sizeof(tstr), "Unknown", NULL);
        }
        if ((W_Total + 2*i + 1) < mainLayout_->count()) {
            getLabel(W_Total + 2*i + 1)->setText(QString(tstr));
        } else {
            mainLayout_->addWidget(new QLabel(tstr), 7 + 2*i, 1, Qt::AlignLeft);
        }

        h += 8 + fm.height();
    }

    resize(QSize(w, h));
    update();
}

}  // namespace debugger
