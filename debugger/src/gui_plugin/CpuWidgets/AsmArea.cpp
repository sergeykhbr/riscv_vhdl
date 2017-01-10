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

namespace debugger {

static const int LINES_MAX = 20;    // remove it

AsmArea::AsmArea(IGui *gui, QWidget *parent)
    : QTableWidget(parent) {
    igui_ = gui;
    isrc_ = 0;
    cmdRegs_.make_string("reg npc");
    cmdReadMem_.make_string("read 0x80000000 20");
    data_.make_data(8);
    tmpBuf_.make_data(1024);
    dataText_.make_string("");
    state_ = CMD_idle;

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
    setRowCount(LINES_MAX); //height() / lineHeight_);
    asmLines_.make_list(LINES_MAX);
    rangeModel_.setMaxLines(LINES_MAX);
    brList_.make_list(0);
    portBrList_ = new BreakpointListPort(igui_, &brList_);
    portBreak_ = new BreakpointInjectPort(igui_, &brList_);
    
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
        item(i, 0)->setTextAlignment(Qt::AlignRight);
    }

    setHorizontalHeaderLabels(
        QString("addr/line;code;label;mnemonic;comment").split(";"));
    setColumnWidth(0, 10 + fm.width(tr("   :0011223344556677")));
    setColumnWidth(1, 10 + fm.width(tr("00112233")));
    setColumnWidth(2, 10 + fm.width(tr("0")));
    setColumnWidth(3, 10 + fm.width(tr("addw    r1,r2,0x112233445566")));

    asmLines_.make_list(0);

    connect(this, SIGNAL(signalHandleResponse()),
            this, SLOT(slotHandleResponse()));
    connect(this, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(slotCellDoubleClicked(int, int)));
}

AsmArea::~AsmArea() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(portBrList_));
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(portBreak_));
    delete portBrList_;
    delete portBreak_;
}

void AsmArea::slotPostInit(AttributeType *cfg) {
    AttributeType src_list;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &src_list);
    if (!src_list.is_list() || src_list.size() == 0) {
        return;
    }
    IService *iserv = static_cast<IService *>(src_list[0u].to_iface());
    isrc_ = static_cast<ISourceCode *>(iserv->getInterface(IFACE_SOURCE_CODE));

    // Prepare commands that don't change in run-time:
    ISocInfo *isoc = static_cast<ISocInfo *>(igui_->getSocInfo());
    DsuMapType *dsu = isoc->getpDsu();

    portBreak_->setBrAddressFetch(
        reinterpret_cast<uint64_t>(&dsu->udbg.v.br_address_fetch));
    portBreak_->setHwRemoveBreakpoint(
        reinterpret_cast<uint64_t>(&dsu->udbg.v.remove_breakpoint));
}

void AsmArea::slotUpdateByTimer() {
    portBrList_->update();      // doesn't use tap interface
    switch (state_) {
    case CMD_idle:
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                                &cmdRegs_, true);
        state_ = CMD_npc;
        break;
    default:;
    }
}

void AsmArea::slotBreakpoint() {
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
    bool br_add = true;
    uint64_t addr = asmLines_[row][COL_addrline].to_uint64();
    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        if (br[0u].to_uint64() == addr) {
            br_add = false;
            RISCV_sprintf(tstr, sizeof(tstr), "br rm 0x%" RV_PRI64 "x", addr);
            break;
        }
    }
    if (br_add) {
        RISCV_sprintf(tstr, sizeof(tstr), "br add 0x%" RV_PRI64 "x", addr);
    }
    AttributeType brcmd(tstr);
    // Do not handle breakpoint responses:
    igui_->registerCommand(NULL, &brcmd, true);
    portBrList_->update();
    rangeModel_.updateAddr(addr);
}

void AsmArea::slotHandleResponse() {
    outLines();
}

void AsmArea::BreakpointListPort::handleResponse(AttributeType *req,
                                                AttributeType *resp) {
    if (!resp->is_list()) {
        return;
    }
    if (resp->size() != p_brList_->size()) {
        (*p_brList_) = (*resp);
        return;
    }
    for (unsigned i = 0; i < resp->size(); i++) {
        const AttributeType &br1 = (*resp)[i];
        AttributeType &br2 = (*p_brList_)[i];
        if (br1[BrkList_address].to_uint64() 
            != br2[BrkList_address].to_uint64()) {
            br2 = br1;
        }
    }
}

void AsmArea::BreakpointInjectPort::handleResponse(AttributeType *req,
                                                   AttributeType *resp) {
    char tstr[128];
    AttributeType memWrite;
    if (!resp->is_integer()) {
        return;
    }
    uint64_t br_addr = resp->to_uint64();
    uint32_t br_instr = 0;
    bool br_hw;
    for (unsigned i = 0; i < (*p_brList_).size(); i++) {
        const AttributeType &br = (*p_brList_)[i];
        if (br_addr == br[BrkList_address].to_uint64()) {
            br_instr = br[BrkList_instr].to_int();
            br_hw = br[BrkList_hwflag].to_bool();
            break;
        }
    }
    if (br_instr == 0) {
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
}

void AsmArea::handleResponse(AttributeType *req, AttributeType *resp) {
    char tstr[128];
    switch (state_) {
    case CMD_idle:
        break;
    case CMD_npc:
        if (!resp->is_integer()) {
            state_ = CMD_idle;
            return;
        }
        rangeModel_.setPC(resp->to_uint64());
        if (rangeModel_.getReqSize() == 0) {
            emit signalHandleResponse();
            state_ = CMD_idle;
            return;
        }
        state_ = CMD_memdata;
        RISCV_sprintf(tstr, sizeof(tstr), "read 0x%08" RV_PRI64 "x %d",
                                          rangeModel_.getReqAddr(),
                                          rangeModel_.getReqSize());
        cmdReadMem_.make_string(tstr);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                                &cmdReadMem_, true);
        break;
    case CMD_memdata:
        if (!resp->is_data()) {
            state_ = CMD_idle;
            return;
        }
        data2lines(rangeModel_.getReqAddr(), *resp, asmLines_);
        rangeModel_.updateRange();
        emit signalHandleResponse();
        state_ = CMD_idle;
        break;
    default:;
    }
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
            item(i, 0)->setTextAlignment(Qt::AlignRight);
        }
    }

    // Output visible assembler lines:
    int sel_row = -1;
    for (unsigned i = 0; i < asmLines_.size(); i++) {
        AttributeType &asmLine = asmLines_[i];
        if (!asmLine.is_list()) {
            continue;
        }
        outLine(static_cast<int>(i), asmLines_[i]);

        if (rangeModel_.getPC() == asmLine[COL_addrline].to_uint64()) {
            sel_row = static_cast<int>(i);
        }
        item(i, 0)->setFlags(item(i, 0)->flags() & ~Qt::ItemIsSelectable);
    }

    // Hide unused lines:
    for (int i = static_cast<int>(asmLines_.size()); i < rowCount(); i++) {
        hideRow(i);
    }

    if (sel_row != -1) {
        QTableWidgetItem *p_item = item(sel_row, 0);
        p_item->setFlags(p_item->flags() | Qt::ItemIsSelectable);
        selectRow(sel_row);
        //scrollTo(sel_row, QAbstractItemView::EnsureVisible);
    }
}

void AsmArea::outLine(int idx, AttributeType &line) {
    char stmp[256];
    QTableWidgetItem *pw;
    uint64_t addr;
    if (idx >= rowCount()) {
        return;
    }
    if (!line.is_list()) {
        return;
    }
    
    addr = line[COL_addrline].to_uint64();
    if (line.size() == 2) {
        pw = item(idx, COL_addrline);
        RISCV_sprintf(stmp, sizeof(stmp), "%d", static_cast<int>(addr));
        pw->setText(QString(stmp));
        pw->setTextColor(QColor(Qt::darkBlue));

        pw = item(idx, COL_code);
        setSpan(idx, COL_code, 1, 4);
        pw->setText(QString(line[COL_code].to_string()));

        item(idx, COL_label)->setText(tr(""));
        item(idx, COL_mnemonic)->setText(tr(""));
        item(idx, COL_comment)->setText(tr(""));
    } else {
        pw = item(idx, COL_addrline);
        RISCV_sprintf(stmp, sizeof(stmp), "%016" RV_PRI64 "x", addr);
        pw->setText(QString(stmp));
        pw->setTextColor(QColor(Qt::black));

        pw = item(idx, COL_code);
        uint32_t instr = static_cast<uint32_t>(line[COL_code].to_uint64());
        if (line[COL_breakpoint].to_bool()) {
            pw->setBackgroundColor(Qt::red);
            pw->setTextColor(Qt::white);
        } else if (pw->backgroundColor() != Qt::lightGray) {
            pw->setBackgroundColor(Qt::lightGray);
            pw->setTextColor(Qt::black);
        }
        RISCV_sprintf(stmp, sizeof(stmp), "%08x", instr);
        pw->setText(QString(stmp));

        pw = item(idx, COL_label);
        pw->setText(QString(line[COL_label].to_string()));

        pw = item(idx, COL_mnemonic);
        pw->setText(QString(line[COL_mnemonic].to_string()));

        pw = item(idx, COL_comment);
        if (line.size() >= COL_comment) {
            pw->setText(QString(line[COL_comment].to_string()));
        } else {
            item(idx, COL_comment)->setText(tr(""));
        }
    }
}

bool AsmArea::addrIsBreakpoint(uint64_t addr, uint8_t *data,
                               AttributeType *obr) {
    uint32_t instr = *reinterpret_cast<uint32_t *>(data);
    if (instr != 0x00100073) {
        return false;
    }
    for (unsigned i = 0; i < brList_.size(); i++) {
        const AttributeType &br = brList_[i];
        if (addr == br[BrkList_address].to_uint64()) {
            *obr = br;
            break;
        }
    }
    return true;
}

int AsmArea::makeAsmAttr(uint64_t addr, uint8_t *data, int offset,
                         AttributeType &out) {
    int ret = 4;
    if (!out.is_list() || out.size() != COL_Total) {
        out.make_list(COL_Total);
    }
    out[COL_addrline].make_uint64(addr + offset);
    out[COL_label].make_string("");
    out[COL_breakpoint].make_boolean(false);
    if (isrc_) {
        AttributeType br;
        uint8_t *pinstr = data + offset;
        if (addrIsBreakpoint(addr + offset, data + offset, &br)) {
            out[COL_breakpoint].make_boolean(true);
            pinstr = br[BrkList_instr].data();
            ret = isrc_->disasm(addr + offset, pinstr, 0,
                                &out[COL_mnemonic],
                                &out[COL_comment]);
        } else {
            ret = isrc_->disasm(addr, data, offset,
                                &out[COL_mnemonic],
                                &out[COL_comment]);
        }
        
        switch (ret) {
        case 2:
            // To support Compressed ISA extension:
            out[COL_code].make_uint64(*reinterpret_cast<uint16_t *>(pinstr));
            break;
        case 4:
            out[COL_code].make_uint64(*reinterpret_cast<uint32_t *>(pinstr));
            break;
        default:
            out[COL_code].make_uint64(~0);
        }
    } else {
        out[COL_mnemonic].make_string("no disassembler");
        out[COL_comment].make_string("");
        out[COL_code].make_uint64(
            *reinterpret_cast<uint32_t *>(&data[offset]));
    }
    out[COL_codesize].make_uint64(ret);
    return ret;
}

void AsmArea::data2lines(uint64_t addr, AttributeType &resp,
                                        AttributeType &lines) {
    unsigned off = 0;
    uint64_t asm_addr_start = 0;
    uint64_t asm_addr_end = 0;
    AttributeType tattr;
    if (lines.size()) {
        asm_addr_start = lines[0u][COL_addrline].to_uint64();
        asm_addr_end = lines[lines.size() - 1][COL_addrline].to_uint64();
    }
    tattr.make_list(COL_Total);
    if (addr + resp.size() == asm_addr_start) {
        // Joint lines at the beginning of current list:
        int idx = 0;
        while (off < resp.size()) {
            off += makeAsmAttr(addr, resp.data(), off, tattr);
            lines.insert_to_list(idx++, &tattr);
        }
    } else if (addr >= asm_addr_start && addr <= asm_addr_end) {
        // Modify lines values:
        for (unsigned i = 0; i < lines.size(); i++) {
            AttributeType &ln = lines[i];
            if ((addr + off) == ln[COL_addrline].to_uint64()) {
                off += makeAsmAttr(addr, resp.data(), off, ln);
            }
            if (off >= resp.size()) {
                break;
            }
        }
    } else if (addr == asm_addr_end + 4) {  // fixme
        // Joint lines to end of the current list:
        while (off < resp.size()) {
            off += makeAsmAttr(addr, resp.data(), off, tattr);
            lines.add_to_list(&tattr);
        }
    } else {
        // Create new list of assembler lines:
        lines.make_list(0);
        while (off < resp.size()) {
            off += makeAsmAttr(addr, resp.data(), off, tattr);
            lines.add_to_list(&tattr);
        }
    }
}

}  // namespace debugger
