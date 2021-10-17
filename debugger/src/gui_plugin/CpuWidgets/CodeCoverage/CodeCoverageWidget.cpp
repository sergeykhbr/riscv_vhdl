/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Code Coverage widgets.
 */

#include "CodeCoverageWidget.h"
#include "CoverageBar.h"
#include "CoverageTable.h"
#include <memory>

namespace debugger {

CodeCoverageWidget::CodeCoverageWidget(IGui *igui, QWidget *parent)
    : QWidget(parent) {
    igui_ = igui;

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);

    CoverageProgressBar *bar = new CoverageProgressBar(this);
    QPushButton *btnDetails = new QPushButton(tr("&Update"), this);
    btnDetails->setFlat(false);
    btnDetails->setCheckable(false);

    lblCoverage_ = new QLabel("0.00");

    CoverageTable *table = new CoverageTable(this);

    gridLayout->addWidget(bar, 0, 0);
    gridLayout->addWidget(lblCoverage_, 0, 1);
    gridLayout->addWidget(btnDetails, 0, 2);
    gridLayout->addWidget(table, 1, 0, 1, 3, Qt::AlignCenter);
    gridLayout->setColumnStretch(0, 10);
    gridLayout->setRowStretch(1, 10);

    setLayout(gridLayout);

    connect(btnDetails, SIGNAL(released()),
            this, SLOT(slotDetailedInfoRequest()));

    connect(this, SIGNAL(signalDetailedInfoUpdate()),
            table, SLOT(slotDatailedInfoUpdate()));

    connect(this, SIGNAL(signalDetailedInfoUpdate()),
            bar, SLOT(slotDatailedInfoUpdate()));

    cmdBriefInfo_.make_string("coverage");
    cmdDetailedInfo_.make_string("coverage detailed");
    requested_ = false;
    requested_detailed_ = false;
}

CodeCoverageWidget::~CodeCoverageWidget() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void CodeCoverageWidget::handleResponse(const char *cmd) {
    if (cmdBriefInfo_.is_equal(cmd)) {
        coverage_ = respBriefInfo_.to_float();
        requested_ = false;
    } else if (cmdDetailedInfo_.is_equal(cmd)) {
        emit signalDetailedInfoUpdate();
        requested_detailed_ = false;
    }
}

void CodeCoverageWidget::slotUpdateByTimer() {
    if (!isVisible()) {
        return;
    }
    if (requested_) {
        return;
    }
    QString total = QString::asprintf("%.2f %%", coverage_);
    lblCoverage_->setText(total);

    requested_ = true;
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                           cmdBriefInfo_.to_string(), &respBriefInfo_, true);
}

void CodeCoverageWidget::slotDetailedInfoRequest() {
    if (!isVisible()) {
        return;
    }
    if (requested_detailed_) {
        return;
    }
    requested_detailed_ = true;
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                    cmdDetailedInfo_.to_string(), &respDetailedInfo_, true);
}

}  // namespace debugger
