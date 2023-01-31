/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer area.
 */

#include "AsmArea.h"
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

namespace debugger {

static const uint64_t MAX_BYTES_VIEW = 1 << 12; // 4096

AsmArea::AsmArea(IGui *gui, QWidget *parent, uint64_t fixaddr)
    : QTableWidget(parent) {
    igui_ = gui;
    cmdReadMem_.make_string("disas 0 0");
    hideLineIdx_ = 0;
    selRowIdx = -1;
    fixaddr_ = fixaddr;
    waitRegNpc_ = false;

    clear();
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    setContentsMargins(QMargins(0, 0, 0, 0));
    QFontMetrics fm(font);
    setMinimumWidth(50 + fm.horizontalAdvance(tr(
    "   :0011223344556677  11223344 1: addw    r0,r1,0xaabbccdd  ; commment")));
    lineHeight_ = fm.height() + 4;
    visibleLinesTotal_ = 0;
    if (isNpcTrackEna()) {
        startAddr_ = 0;
        endAddr_ = 0;
    } else {
        startAddr_ = fixaddr_;
        endAddr_ = fixaddr_;  // will be computed in resize() method
    }

    setColumnCount(COL_Total);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setRowCount(1);
    asmLines_.make_list(0);
    asmLinesOut_.make_list(0);
    npc_ = ~0;
    
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
        item(i, 0)->setBackground(QColor(Qt::lightGray));
        item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    setHorizontalHeaderLabels(
        QString("addr/line;code;label;mnemonic;comment").split(";"));
    setColumnWidth(0, 10 + fm.horizontalAdvance(tr("   :0011223344556677")));
    setColumnWidth(1, 10 + fm.horizontalAdvance(tr("00112233")));
    setColumnWidth(2, 10 + fm.horizontalAdvance(tr("0")));
    setColumnWidth(3, 10 + fm.horizontalAdvance(tr("addw    r1,r2,0x112233445566")));
    setColumnWidth(4, 10 + fm.horizontalAdvance(tr("some very long long comment+offset")));

    connect(this, SIGNAL(signalNpcChanged()),
            this, SLOT(slotNpcChanged()));
    connect(this, SIGNAL(signalAsmListChanged()),
            this, SLOT(slotAsmListChanged()));
    connect(this, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(slotCellDoubleClicked(int, int)));

    RISCV_mutex_init(&mutexAsmGaurd_);

    reqNpc_.make_string("reg pc");
}

AsmArea::~AsmArea() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
    RISCV_mutex_destroy(&mutexAsmGaurd_);
}

void AsmArea::resizeEvent(QResizeEvent *ev) {
    int h = ev->size().height();
    if (h < 4 * lineHeight_) {
        h = 4 * lineHeight_;
    }
    visibleLinesTotal_ = h / lineHeight_ + 2;

    if (isNpcTrackEna() && npc_ == ~0ull) {
        return;
    }

    int line_cnt = static_cast<int>(asmLinesOut_.size());
    if (visibleLinesTotal_ > line_cnt) {
        char tstr[256];
        RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                    endAddr_,
                    4*(visibleLinesTotal_ - line_cnt));
        cmdReadMem_.make_string(tstr);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                               cmdReadMem_.to_string(), &respReadMem_, true);
    }

    QWidget::resizeEvent(ev);
}

void AsmArea::wheelEvent(QWheelEvent * ev) {
    QPoint dlt = ev->angleDelta();
    int scroll_pos = verticalScrollBar()->value();
    int scroll_min = verticalScrollBar()->minimum();
    int scroll_max = verticalScrollBar()->maximum();
    if (dlt.y() >= 0 && scroll_pos > scroll_min) {
        verticalScrollBar()->setValue(--scroll_pos);
    } else if (dlt.y() < 0 && scroll_pos < scroll_max) {
        verticalScrollBar()->setValue(++scroll_pos);
    } else {
        char tstr[128];
        unsigned sz = 4 * static_cast<unsigned>(visibleLinesTotal_ / 2);
        if (dlt.y() >= 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                        startAddr_ - sz, sz);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                        endAddr_, sz);
        }
        cmdReadMem_.make_string(tstr);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                               cmdReadMem_.to_string(), &respReadMem_, true);
    }
    QWidget::wheelEvent(ev);
}

void AsmArea::slotUpdateByTimer() {
    if (waitRegNpc_) {
        return;
    }
    waitRegNpc_ = true;
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                            reqNpc_.to_string(), &respNpc_, true);
}

void AsmArea::handleResponse(const char *cmd) {
    if (reqNpc_.is_equal(cmd)) {
        waitRegNpc_ = false;
        if (!respNpc_.is_nil()) {
            npc_ = respNpc_.to_uint64();
            emit signalNpcChanged();
        }
    } else if (strstr(cmd, "br ")) {
        emit signalBreakpointsChanged();
    } else if (strstr(cmd, "disas") && !respReadMem_.is_nil()) {
        addMemBlock(respReadMem_, asmLines_);
        emit signalAsmListChanged();
    }
}

bool AsmArea::isNpcTrackEna() {
    return (fixaddr_ == ~0ull);
}

void AsmArea::slotNpcChanged() {
    int npc_row = getNpcRowIdx();
    if (npc_row != -1) {
        selectNpcRow(npc_row);
        return;
    }
    if (!isNpcTrackEna()) {
        return;
    }

    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                npc_, 4*visibleLinesTotal_);
    cmdReadMem_.make_string(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                            cmdReadMem_.to_string(), &respReadMem_, true);
}

void AsmArea::slotAsmListChanged() {
    RISCV_mutex_lock(&mutexAsmGaurd_);
    asmLinesOut_ = asmLines_;
    RISCV_mutex_unlock(&mutexAsmGaurd_);

    adjustRowCount();
    clearSpans();

    if (asmLinesOut_.size() == 0) {
        return;
    }

    bool reinit_start = true;
    for (unsigned i = 0; i < asmLinesOut_.size(); i++) {
        AttributeType &asmLine = asmLinesOut_[i];
        if (!asmLine.is_list()) {
            continue;
        }
        switch (asmLine[ASM_list_type].to_uint32()) {
        case AsmList_symbol:
            outSymbolLine(static_cast<int>(i), asmLine);
            break;
        case AsmList_disasm:
            endAddr_ = asmLine[ASM_addrline].to_uint64();
            if (reinit_start) {
                reinit_start = false;
                startAddr_ = endAddr_;
            }
            endAddr_ += asmLine[ASM_codesize].to_uint64();
            outAsmLine(static_cast<int>(i), asmLine);
            break;
        default:;
        }
    }
}

void AsmArea::slotRedrawDisasm() {
    char tstr[256];
    uint64_t sz = endAddr_ - startAddr_;
    if (sz == 0) {
        return;
    }
    if (sz > MAX_BYTES_VIEW) {
        sz = MAX_BYTES_VIEW;
    }
    RISCV_sprintf(tstr, sizeof(tstr), "disas 0x%" RV_PRI64 "x %d",
                startAddr_, sz);
    cmdReadMem_.make_string(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                            cmdReadMem_.to_string(), &respReadMem_, true);
}

void AsmArea::slotCellDoubleClicked(int row, int column) {
    if (row >= static_cast<int>(asmLinesOut_.size())) {
        return;
    }
    AttributeType &asmLine = asmLinesOut_[row];
    if (!asmLine.is_list()
        || asmLine[ASM_list_type].to_uint32() != AsmList_disasm) {
        return;
    }
    char tstr[128];
    bool is_breakpoint = asmLine[ASM_breakpoint].to_bool();
    uint64_t addr = asmLine[ASM_addrline].to_uint64();

    if (is_breakpoint) {
        RISCV_sprintf(tstr, sizeof(tstr), "br rm 0x%" RV_PRI64 "x", addr);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "br add 0x%" RV_PRI64 "x", addr);
    }
    AttributeType brcmd(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           brcmd.to_string(), &respBr_, true);
}

int AsmArea::getNpcRowIdx() {
    for (unsigned i = 0; i < asmLinesOut_.size(); i++) {
        AttributeType &asmLine = asmLinesOut_[i];
        if (!asmLine.is_list()) {
            continue;
        }
        if (asmLine[ASM_list_type].to_uint32() != AsmList_disasm) {
            continue;
        }
        if (npc_ == asmLine[ASM_addrline].to_uint64()) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void AsmArea::selectNpcRow(int idx) {
    QTableWidgetItem *p;
    if (selRowIdx != -1 && selRowIdx != idx) {
        p = item(selRowIdx, 0);
        p->setFlags(p->flags() & ~Qt::ItemIsSelectable);
    }
    selRowIdx = idx;
    p = item(selRowIdx, 0);
    p->setFlags(p->flags() | Qt::ItemIsSelectable);
    selectRow(selRowIdx);
}

void AsmArea::adjustRowCount() {
    int asm_cnt = static_cast<int>(asmLinesOut_.size());
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
            item(i, 0)->setBackground(QColor(Qt::lightGray));
            item(i, 0)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
    } 
    if (hideLineIdx_ < asm_cnt) {
        for (int i = hideLineIdx_; i < asm_cnt; i++) {
            showRow(i);
        }
    }
    hideLineIdx_ = asm_cnt;
    for (int i = hideLineIdx_; i < rowCount(); i++) {
        hideRow(i);
    }
}

void AsmArea::outSymbolLine(int idx, AttributeType &line) {
    QTableWidgetItem *pw;
    uint64_t addr = line[ASM_addrline].to_uint64();
    setSpan(idx, COL_label, 1, 3);

    pw = item(idx, COL_addrline);
    pw->setText(QString("<%1>").arg(addr, 16, 16, QChar('0')));
    pw->setForeground(QColor(QColor(Qt::darkBlue)));

    pw = item(idx, COL_code);
    pw->setBackground(QColor(Qt::lightGray));
    pw->setText(tr(""));

    pw = item(idx, COL_label);
    pw->setText(QString(line[ASM_code].to_string()));
    pw->setForeground(QColor(QColor(Qt::darkBlue)));

    item(idx, COL_mnemonic)->setText(tr(""));
    item(idx, COL_comment)->setText(tr(""));
}

void AsmArea::outAsmLine(int idx, AttributeType &line) {
    QTableWidgetItem *pw;
    uint64_t addr = line[ASM_addrline].to_uint64();
    if (addr == npc_) {
        selectNpcRow(idx);
    }

    pw = item(idx, COL_addrline);
    pw->setText(QString("%1").arg(addr, 16, 16, QChar('0')));
    pw->setForeground(QColor(QColor(Qt::black)));
    pw->setBackground(QColor(Qt::lightGray));

    pw = item(idx, COL_code);
    uint32_t instr = line[ASM_code].to_uint32();
    if (line[ASM_breakpoint].to_bool()) {
        pw->setBackground(QColor(Qt::red));
        pw->setForeground(QColor(Qt::white));
    } else if (pw->background().color() != Qt::lightGray) {
        pw->setBackground(QColor(Qt::lightGray));
        pw->setForeground(QColor(Qt::black));
    }
    int codesz = line[ASM_codesize].to_int();
    pw->setText(QString("%1").arg(instr, 2*codesz, 16, QChar('0')));

    pw = item(idx, COL_label);
    pw->setForeground(QColor(Qt::black));
    pw->setText(QString(line[ASM_label].to_string()));

    pw = item(idx, COL_mnemonic);
    pw->setText(QString(line[ASM_mnemonic].to_string()));

    pw = item(idx, COL_comment);
    pw->setText(QString(line[ASM_comment].to_string()));
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
