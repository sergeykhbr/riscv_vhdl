/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer area.
 */

#include "AsmArea.h"
#include "moc_AsmArea.h"

#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

namespace debugger {

AsmArea::AsmArea(IGui *gui, QWidget *parent, uint64_t fixaddr)
    : QTableWidget(parent) {
    igui_ = gui;
    cmdRegs_.make_string("reg npc");
    cmdReadMem_.make_string("disas 0 0");
    state_ = CMD_idle;
    hideLineIdx_ = 0;
    selRowIdx = -1;
    fixaddr_ = fixaddr;

    clear();
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    setContentsMargins(QMargins(0, 0, 0, 0));
    QFontMetrics fm(font);
    setMinimumWidth(50 + fm.width(tr(
    "   :0011223344556677  11223344 1: addw    r0,r1,0xaabbccdd  ; commment")));
    lineHeight_ = fm.height() + 4;
    portBreak_ = 0;

    setColumnCount(COL_Total);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setRowCount(1);
    asmLines_.make_list(0);
    rangeModel_.setMaxLines(4);
    portBreak_ = new BreakpointInjectPort(igui_);
    
    verticalHeader()->setVisible(false);        // remove row indexes
    setShowGrid(false);                         // remove borders
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows); //select full row

    // change selected row color
    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Highlight, Qt::gray);
    setPalette(*palette);
    Qt::ItemFlags fl;
    for (int i = 0; i < rowCount(); i++) {
        for (int n = 0; n < COL_Total; n++) {
            setItem(i, n, new QTableWidgetItem());
            fl = item(i, n)->flags();
            fl &= ~Qt::ItemIsEditable;
            fl &= ~Qt::ItemIsSelectable;
            item(i, n)->setFlags(fl);
        }
        setRowHeight(i, lineHeight_);
        item(i, 0)->setBackgroundColor(Qt::lightGray);
        item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    setHorizontalHeaderLabels(
        QString("addr/line;code;label;mnemonic;comment").split(";"));
    setColumnWidth(0, 10 + fm.width(tr("   :0011223344556677")));
    setColumnWidth(1, 10 + fm.width(tr("00112233")));
    setColumnWidth(2, 10 + fm.width(tr("0")));
    setColumnWidth(3, 10 + fm.width(tr("addw    r1,r2,0x112233445566")));
    setColumnWidth(4, 10 + fm.width(tr("some very long long comment+offset")));

    connect(this, SIGNAL(signalNpcChanged(uint64_t)),
            this, SLOT(slotNpcChanged(uint64_t)));
    connect(this, SIGNAL(signalAsmListChanged()),
            this, SLOT(slotAsmListChanged()));
    connect(this, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(slotCellDoubleClicked(int, int)));

    RISCV_mutex_init(&mutexAsmGaurd_);
    initSocAddresses();
}

AsmArea::~AsmArea() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(portBreak_));
    RISCV_mutex_destroy(&mutexAsmGaurd_);
    delete portBreak_;
}

void AsmArea::initSocAddresses() {
    // Prepare commands that don't change in run-time:
    ISocInfo *isoc = static_cast<ISocInfo *>(igui_->getSocInfo());
    if (!isoc) {
        return;
    }
    DsuMapType *dsu = isoc->getpDsu();

    portBreak_->setBrAddressFetch(
        reinterpret_cast<uint64_t>(&dsu->udbg.v.br_address_fetch));
    portBreak_->setHwRemoveBreakpoint(
        reinterpret_cast<uint64_t>(&dsu->udbg.v.remove_breakpoint));
}

void AsmArea::resizeEvent(QResizeEvent *ev) {
    int h = ev->size().height();
    if (h < 4 * lineHeight_) {
        h = 4 * lineHeight_;
    }
    rangeModel_.setMaxLines(h / lineHeight_ + 2);
    QWidget::resizeEvent(ev);
    //scrollToItem(item(0,0));

    if (fixaddr_ != ~0ull) {
        emit signalNpcChanged(fixaddr_);
    }
}

void AsmArea::wheelEvent(QWheelEvent * ev) {
    int dlt = ev->delta();
    int scroll_pos = verticalScrollBar()->value();
    int scroll_min = verticalScrollBar()->minimum();
    int scroll_max = verticalScrollBar()->maximum();
    if (dlt >= 0 && scroll_pos > scroll_min) {
        verticalScrollBar()->setValue(--scroll_pos);
    } else if (dlt < 0 && scroll_pos < scroll_max) {
        verticalScrollBar()->setValue(++scroll_pos);
    } else {
        rangeModel_.insrease(dlt);
        if (state_ != CMD_idle) {
            return;
        }
        char tstr[128];
        state_ = CMD_memdata;
        RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                    rangeModel_.getReqAddr(),
                    rangeModel_.getReqSize());
        cmdReadMem_.make_string(tstr);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                                &cmdReadMem_, true);
    }
    QWidget::wheelEvent(ev);
}

void AsmArea::slotPostInit(AttributeType *cfg) {
    initSocAddresses();
}

void AsmArea::slotUpdateByTimer() {
    if (state_ ==  CMD_idle) {
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                                &cmdRegs_, true);
        state_ = CMD_npc;
    }
}

void AsmArea::handleResponse(AttributeType *req, AttributeType *resp) {
    if (req->is_equal("reg npc")) {
        emit signalNpcChanged(resp->to_uint64());
    } else if (strstr(req->to_string(), "br ")) {
        emit signalBreakpointsChanged();
    } else if (strstr(req->to_string(), "disas")) {
        addMemBlock(*resp, asmLines_);
        emit signalAsmListChanged();
    }
}

void AsmArea::slotNpcChanged(uint64_t npc) {
    int sel_row_new = -1;

    RISCV_mutex_lock(&mutexAsmGaurd_);
    for (unsigned i = 0; i < asmLines_.size(); i++) {
        AttributeType &asmLine = asmLines_[i];
        if (!asmLine.is_list()) {
            continue;
        }
        if (npc == asmLine[ASM_addrline].to_uint64()) {
            sel_row_new = static_cast<int>(i);
            break;
        }
    }
    RISCV_mutex_unlock(&mutexAsmGaurd_);

    QTableWidgetItem *p;
    if (sel_row_new != -1) {
        if (selRowIdx != -1 && selRowIdx != sel_row_new) {
            p = item(selRowIdx, 0);
            p->setFlags(p->flags() & ~Qt::ItemIsSelectable);
        }
        selRowIdx = sel_row_new;
        p = item(selRowIdx, 0);
        p->setFlags(p->flags() | Qt::ItemIsSelectable);
        selectRow(selRowIdx);
        state_ = CMD_idle;
    } else {
        char tstr[128];
        if (selRowIdx != -1) {
            p = item(selRowIdx, 0);
            p->setFlags(p->flags() & ~Qt::ItemIsSelectable);
        }

        rangeModel_.setPC(npc);
        RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                    rangeModel_.getReqAddr(),
                    rangeModel_.getReqSize());
        cmdReadMem_.make_string(tstr);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                                &cmdReadMem_, true);
        state_ = CMD_memdata;
    }
}

void AsmArea::slotAsmListChanged() {
    rangeModel_.updateRange();
    outLines();
    state_ = CMD_idle;
}

void AsmArea::slotBreakpointHalt() {
    portBreak_->injectFetch();
}

void AsmArea::slotRedrawDisasm() {
    portBreak_->injectFetch();
}

void AsmArea::slotCellDoubleClicked(int row, int column) {
    if (row >= static_cast<int>(asmLines_.size())) {
        return;
    }
    if (!asmLines_[row].is_list() || !asmLines_[row].size()) {
        return;
    }
    char tstr[128];
    bool is_breakpoint = asmLines_[row][ASM_breakpoint].to_bool();
    uint64_t addr = asmLines_[row][ASM_addrline].to_uint64();

    if (is_breakpoint) {
        RISCV_sprintf(tstr, sizeof(tstr), "br rm 0x%" RV_PRI64 "x", addr);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "br add 0x%" RV_PRI64 "x", addr);
    }
    AttributeType brcmd(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &brcmd, true);

    // Update disassembler list:
    rangeModel_.updateAddr(addr);
    RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                rangeModel_.getReqAddr(),
                rangeModel_.getReqSize());
    cmdReadMem_.make_string(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                            &cmdReadMem_, true);
}

void AsmArea::BreakpointInjectPort::injectFetch() {
    if (is_waiting_) {
        return;
    }
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            &readBr_, true);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            &readNpc_, true);
    is_waiting_ = true;
}

void AsmArea::BreakpointInjectPort::handleResponse(AttributeType *req,
                                                   AttributeType *resp) {
    char tstr[128];
    AttributeType memWrite;
    if (req->is_equal("br")) {
        brList_ = *resp;
        return;
    }
    uint64_t br_addr = resp->to_uint64();
    uint32_t br_instr = 0;
    bool br_hw;
    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        if (br_addr == br[BrkList_address].to_uint64()) {
            br_instr = br[BrkList_instr].to_int();
            br_hw = br[BrkList_hwflag].to_bool();
            break;
        }
    }
    if (br_instr == 0) {
        is_waiting_ = false;
        return;
    }
    if (br_hw) {
        RISCV_sprintf(tstr, sizeof(tstr),
                "write 0x%08" RV_PRI64 "x 8 0x%" RV_PRI64 "x",
                dsu_hw_br_, br_addr);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr),
                "write 0x%08" RV_PRI64 "x 16 [0x%" RV_PRI64 "x,0x%x]",
                dsu_sw_br_, br_addr, br_instr);
    }
    memWrite.make_string(tstr);
    igui_->registerCommand(NULL, &memWrite, true);
    is_waiting_ = false;
}

void AsmArea::outLines() {
    int asm_cnt = static_cast<int>(asmLines_.size());
    int row_cnt = rowCount();
    if (row_cnt < asm_cnt) {
        Qt::ItemFlags fl;
        setRowCount(asm_cnt);
        for (int i = row_cnt; i < asm_cnt; i++) {
            for (int n = 0; n < COL_Total; n++) {
                setItem(i, n, new QTableWidgetItem());
                fl = item(i, n)->flags();
                fl &= ~Qt::ItemIsEditable;
                fl &= ~Qt::ItemIsSelectable;
                item(i, n)->setFlags(fl);
            }
            setRowHeight(i, lineHeight_);
            item(i, 0)->setBackgroundColor(Qt::lightGray);
            item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    } 
    if (hideLineIdx_ < asm_cnt) {
        for (int i = hideLineIdx_; i < asm_cnt; i++) {
            showRow(i);
        }
        hideLineIdx_ = asm_cnt;
    }

    // Output visible assembler lines:
    for (unsigned i = 0; i < asmLines_.size(); i++) {
        AttributeType &asmLine = asmLines_[i];
        if (!asmLine.is_list()) {
            continue;
        }
        outLine(static_cast<int>(i), asmLines_[i]);

        if (rangeModel_.getPC() == asmLine[ASM_addrline].to_uint64()) {
            selRowIdx = static_cast<int>(i);
        }
    }

    // Hide unused lines:
    hideLineIdx_ = static_cast<int>(asmLines_.size());
    for (int i = hideLineIdx_; i < rowCount(); i++) {
        hideRow(i);
    }

    if (selRowIdx != -1) {
        QTableWidgetItem *p_item = item(selRowIdx, 0);
        p_item->setFlags(p_item->flags() | Qt::ItemIsSelectable);
        selectRow(selRowIdx);
    }
}

void AsmArea::outLine(int idx, AttributeType &line) {
    QTableWidgetItem *pw;
    uint64_t addr;
    if (idx >= rowCount()) {
        return;
    }
    if (!line.is_list()) {
        return;
    }
    
    addr = line[ASM_addrline].to_uint64();
    if (line[ASM_list_type].to_int() == AsmList_symbol) {
        setSpan(idx, COL_label, 1, 3);

        pw = item(idx, COL_addrline);
        pw->setText(QString("<%1>").arg(addr, 16, 16, QChar('0')));
        pw->setTextColor(QColor(Qt::darkBlue));

        pw = item(idx, COL_code);
        pw->setBackgroundColor(Qt::lightGray);
        pw->setText(tr(""));

        pw = item(idx, COL_label);
        pw->setText(QString(line[ASM_code].to_string()));
        pw->setTextColor(QColor(Qt::darkBlue));

        item(idx, COL_mnemonic)->setText(tr(""));
        item(idx, COL_comment)->setText(tr(""));
    } else if (line[ASM_list_type].to_int() == AsmList_disasm) {
        setSpan(idx, COL_label, 1, 1);

        pw = item(idx, COL_addrline);
        pw->setText(QString("%1").arg(addr, 16, 16, QChar('0')));
        pw->setTextColor(QColor(Qt::black));
        pw->setBackgroundColor(Qt::lightGray);

        pw = item(idx, COL_code);
        uint32_t instr = line[ASM_code].to_uint32();
        if (line[ASM_breakpoint].to_bool()) {
            pw->setBackgroundColor(Qt::red);
            pw->setTextColor(Qt::white);
        } else if (pw->backgroundColor() != Qt::lightGray) {
            pw->setBackgroundColor(Qt::lightGray);
            pw->setTextColor(Qt::black);
        }
        pw->setText(QString("%1").arg(instr, 8, 16, QChar('0')));

        pw = item(idx, COL_label);
        pw->setTextColor(Qt::black);
        pw->setText(QString(line[ASM_label].to_string()));

        pw = item(idx, COL_mnemonic);
        pw->setText(QString(line[ASM_mnemonic].to_string()));

        pw = item(idx, COL_comment);
        pw->setText(QString(line[ASM_comment].to_string()));
    }
}

void AsmArea::addMemBlock(AttributeType &resp,
                          AttributeType &lines) {
    uint64_t asm_addr_start = 0;
    uint64_t asm_addr_end = 0;
    if (!resp.is_list() || !resp.size()) {
        return;
    }
    if (!resp[0u].is_list() || !resp[resp.size() - 1].is_list()) {
        return;
    }
    if (lines.size()) {
        asm_addr_start = lines[0u][ASM_addrline].to_uint64();
        asm_addr_end = lines[lines.size() - 1][ASM_addrline].to_uint64();
    }
    uint64_t resp_addr_start = resp[0u][ASM_addrline].to_uint64();
    uint64_t resp_addr_next = resp[resp.size() - 1][ASM_addrline].to_uint64();
    resp_addr_next += resp[resp.size() - 1][ASM_codesize].to_uint64();

    RISCV_mutex_lock(&mutexAsmGaurd_);
    if (resp_addr_next == asm_addr_start) {
        // Joint lines at the beginning of current list:
        for (unsigned i = 0; i < resp.size(); i++) {
            lines.insert_to_list(i, &resp[i]);
        }
    } else if (resp_addr_start >= asm_addr_start
            && resp_addr_start <= asm_addr_end) {
        // Modify lines values:
        unsigned resp_idx = 0;
        for (unsigned i = 0; i < lines.size(); i++) {
            AttributeType &ln = lines[i];
            if (resp_addr_start == ln[ASM_addrline].to_uint64()) {
                ln = resp[resp_idx++];
                if (resp_idx < resp.size()) {
                    resp_addr_start = resp[resp_idx][ASM_addrline].to_uint64();
                } else {
                    break;
                }
            }
        }
        for (unsigned i = resp_idx; i < resp.size(); i++) {
            lines.add_to_list(&resp[i]);
        }
    } else if (resp_addr_start == asm_addr_end + 4) {  // fixme
        // Joint lines to end of the current list:
        for (unsigned i = 0; i < resp.size(); i++) {
            lines.add_to_list(&resp[i]);
        }
    } else {
        lines = resp;
    }
    RISCV_mutex_unlock(&mutexAsmGaurd_);
}

}  // namespace debugger
