#include "RegWidget.h"
#include "RegsViewWidget.h"
#include "moc_RegsViewWidget.h"

#include <memory>

namespace debugger {

/** Layout of register by name */
static const char *REG_NAMES_LAYOUT[] = {
    "ra", "s0",  "a0",
    "sp", "s1",  "a1",
    "gp", "s2",  "a2",
    "tp", "s3",  "a3",
    ""  , "s4",  "a4",
    "t0", "s5",  "a5",
    "t1", "s6",  "a6",
    "t2", "s7",  "a7",
    "t3", "s8",  "",
    "t4", "s9",  "",
    "t5", "s10", "pc",
    "t6", "s11", "npc"
};

static const uint64_t BASE_ADDR_DSU_REGS = 0x80090200;


RegsViewWidget::RegsViewWidget(IGui *igui, QWidget *parent) 
    : UnclosableWidget(parent) {
    igui_ = igui;

    setWindowTitle(tr("Registers"));
    setMinimumWidth(150);
    setMinimumHeight(100);

    pollingTimer_ = new QTimer(this);
    pollingTimer_->setSingleShot(false);
    connect(pollingTimer_, SIGNAL(timeout()), this, SLOT(slotPollingUpdate()));

    gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(4);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setContentsMargins(4, 4, 4, 4);
    setLayout(gridLayout);

    minSizeApplied_ = false;

    qRegisterMetaType<uint64_t>("uint64_t");
}


void RegsViewWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    uint64_t reg_idx = ((*req)[1].to_uint64() - BASE_ADDR_DSU_REGS) / 8;
    emit signalRegisterValue(reg_idx, resp->to_uint64());
}

void RegsViewWidget::slotConfigure(AttributeType *cfg) {
    for (unsigned i = 0; i < cfg->size(); i++) {
        const AttributeType &attr = (*cfg)[i];
        if (strcmp(attr[0u].to_string(), "RegList") == 0) {
            listRegs_ = attr[1];
            for (unsigned n = 0; n < listRegs_.size(); n++) {
                addRegWidget(listRegs_[n]);
            }
         
        } else if (strcmp(attr[0u].to_string(), "PollingMs") == 0) {
            int ms = static_cast<int>(attr[1].to_uint64());
            if (ms != 0) {
                pollingTimer_->start(ms);
            }
        }
    }
}

void RegsViewWidget::slotPollingUpdate() {
}

void RegsViewWidget::slotTargetStateChanged(bool running) {
    /** DSU control region 0x80080000 + 0x10000: 
     *              offset 0x0200 = CPU registers
     */
    AttributeType cmdRdReg;
    cmdRdReg.from_config("['read',0x80090200,8]");
    for (unsigned i = 0; i < 32; i++) {
        cmdRdReg[1].make_uint64(BASE_ADDR_DSU_REGS + 0x8 * i);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                               &cmdRdReg);
    }

}

int RegsViewWidget::widgetIndexByName(const char *regname) {
    unsigned regs_total = sizeof(REG_NAMES_LAYOUT)/sizeof(const char *);
    for (unsigned i = 0; i < regs_total; i++) {
        if (strcmp(regname, REG_NAMES_LAYOUT[i]) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void RegsViewWidget::addRegWidget(AttributeType &reg_cfg) {
    QWidget *pnew;

    int idx = widgetIndexByName(reg_cfg[0u].to_string());
    if (idx == -1) {
        return;
    }
    int line = idx / 3;
    int col = idx - 3 * line;

    pnew = new RegWidget(&reg_cfg, this);
    gridLayout->addWidget(pnew, line + 1, col);
    connect(this, SIGNAL(signalRegisterValue(uint64_t, uint64_t)), 
            pnew, SLOT(slotRegisterValue(uint64_t, uint64_t)));

    AttributeType item_cfg;
    item_cfg.make_list(2);

    if (!minSizeApplied_) {
        minSizeApplied_ = true;
        emit signalResize(QSize(3 * (pnew->minimumWidth() + 10), 
                          12 * (pnew->minimumHeight() + 5)));
    }
}


}  // namespace debugger
