/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "PnpWidget.h"
#include <QtGui/QPainter>
#include <QtWidgets/QScrollBar>
#include <memory>

namespace debugger {

enum EWName {
    WTarget_Name,
    WTarget_image,
    WHW_ID,
    WFW_ID,
    WDevicesTotal,
    W_Total
};

PnpWidget::PnpWidget(IGui *igui, QWidget *parent) : QWidget(parent) {
    igui_ = igui;
    parent_ = parent;

    mainLayout_ = new QGridLayout(this);

    setMinimumWidth(150);
    setMinimumHeight(100);

    imgTarget_[SIMULATION] = QImage(tr(":/images/ml605_top.png"));
    imgTarget_[ML605] = QImage(tr(":/images/ml605_top.png"));
    imgTarget_[KC705] = QImage(tr(":/images/kc705_top.png"));
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

    char tstr[64];
    const AttributeType &cfg = *igui->getpConfig();
    RISCV_sprintf(tstr, sizeof(tstr),
                "read 0x%08" RV_PRI64 "x %d",
                cfg["AddrPNP"].to_uint64(),
                sizeof(PnpMapType));
    reqcmd_.make_string(tstr);
}

PnpWidget::~PnpWidget() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void PnpWidget::showEvent(QShowEvent *event_) {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
            reqcmd_.to_string(), &respcmd_, true);
    
    QWidget::showEvent(event_);
}

void PnpWidget::handleResponse(const char *cmd) {
    if (!respcmd_.is_data()) {
        return;
    }
    memcpy(&pnp_, respcmd_.data(), respcmd_.size());
    emit signalUpdate();
}

void PnpWidget::slotUpdate() {
    char tstr[256];
    QPixmap pixmapBkg;
    QString targetText;

    /*switch (pnp_.tech.bits.tech) {
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
    getLabel(WTarget_image)->setPixmap(pixmapBkg);*/

    QFontMetrics fm(font());
    int h = pixmapBkg.size().height() + fm.height() + 50;
    int w = pixmapBkg.size().width();

    RISCV_sprintf(tstr, sizeof(tstr), "HW_ID: 0x%08x", pnp_.hwid);
    getLabel(WHW_ID)->setText(QString(tstr));
    w += 2 * fm.horizontalAdvance(QString(tstr));

    RISCV_sprintf(tstr, sizeof(tstr), "FW_ID: 0x%08x", pnp_.fwid);
    getLabel(WFW_ID)->setText(QString(tstr));
    
    RISCV_sprintf(tstr, sizeof(tstr), "DEV Total: %d", pnp_.cfg.bits.cfg_slots);
    getLabel(WDevicesTotal)->setText(QString(tstr));

    /**
     * Lines with index 5 and 6 are empty.
     */
    DeviceDescriptorType *pdev;
    uint32_t adr1, adr2;
    uint64_t did;
    int i = 0;
    iter_.buf = pnp_.cfg_table;
    while (iter_.item->descr.bits.descrtype != PNP_CFG_TYPE_INVALID) {
        pdev = iter_.item;
        if (iter_.item->descr.bits.descrtype == PNP_CFG_TYPE_MASTER) {
            did = pdev->did;
            RISCV_sprintf(tstr, sizeof(tstr), "mst: ", NULL);
        } else {
            adr1 = pdev->addr_start;
            adr2 = pdev->addr_end;
            did = pdev->did;
            RISCV_sprintf(tstr, sizeof(tstr), "slv: 0x%08x .. 0x%08x", adr1, adr2);
        }
        if ((W_Total + 2*i) < mainLayout_->count()) {
            getLabel(W_Total + 2*i)->setText(QString(tstr));
        } else {
            mainLayout_->addWidget(new QLabel(tstr), 7 + 2*i, 0, Qt::AlignLeft);
        }

        switch (did) {
        case MST_DID_EMPTY:
            RISCV_sprintf(tstr, sizeof(tstr), "Empty master slot", NULL);
            break;
        case RISCV_CACHED_TILELINK:
            RISCV_sprintf(tstr, sizeof(tstr), "Rocket CPU Cached tile", NULL);
            break;
        case RISCV_UNCACHED_TILELINK:
            RISCV_sprintf(tstr, sizeof(tstr), "Rocket CPU Uncached tile", NULL);
            break;
        case GAISLER_ETH_MAC_MASTER:
            RISCV_sprintf(tstr, sizeof(tstr), "Ethernet MAC with DMA interface", NULL);
            break;
        case GAISLER_ETH_EDCL_MASTER:
            RISCV_sprintf(tstr, sizeof(tstr), "EDCL debug with DMA interface", NULL);
            break;
        case RISCV_RIVER_CPU:
            RISCV_sprintf(tstr, sizeof(tstr), "Risc-V RIVER CPU", NULL);
            break;
        case GNSSSENSOR_UART_TAP:
            RISCV_sprintf(tstr, sizeof(tstr), "AXI UART TAP with DMA", NULL);
            break;
        case SLV_DID_EMPTY:
            RISCV_sprintf(tstr, sizeof(tstr), "Empty slave slot", NULL);
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
        case GNSSSENSOR_FSE_V2_GPS:
            RISCV_sprintf(tstr, sizeof(tstr), "GPS Fast Search Engine", NULL);
            break;
        case GNSSSENSOR_UART:
            RISCV_sprintf(tstr, sizeof(tstr), "UART", NULL);
            break;
        case GNSSSENSOR_GPIO:
            RISCV_sprintf(tstr, sizeof(tstr), "GPIO/LEDs", NULL);
            break;
        case GNSSSENSOR_RF_CONTROL:
            RISCV_sprintf(tstr, sizeof(tstr), "RF fron-end Controller", NULL);
            break;
        case GNSSSENSOR_ENGINE_STUB:
            RISCV_sprintf(tstr, sizeof(tstr), "GNSS Enginge stub", NULL);
            break;
        case GNSSSENSOR_ETHMAC:
            RISCV_sprintf(tstr, sizeof(tstr), "Ethernet MAC 10/100", NULL);
            break;
        case GNSSSENSOR_DSU:
            RISCV_sprintf(tstr, sizeof(tstr), "Debug Support Unit", NULL);
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
        i++;
        iter_.buf += iter_.item->descr.bits.descrsize;
    }

    parent_->resize(QSize(w, h));
    update();
}

}  // namespace debugger
