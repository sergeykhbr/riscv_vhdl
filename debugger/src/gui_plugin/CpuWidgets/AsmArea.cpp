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

AsmArea::AsmArea(IGui *gui, QWidget *parent)
    : QTableWidget(parent) {
    igui_ = gui;
    cmdRegs_.make_string("reg npc");
    cmdReadMem_.make_string("read 0x80000000 20");
    data_.make_data(8);
    tmpBuf_.make_data(1024);
    dataText_.make_string("");
    isrc_ = 0;

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
    addrStart_ = ~0;
    addrSize_ = 0;

    setColumnCount(COL_Total);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setRowCount(20); //height() / lineHeight_);
    asmLines_.make_list(20);
    
    verticalHeader()->setVisible(false);        // remove row indexes
    setShowGrid(false);                         // remove borders
    setSelectionBehavior(QAbstractItemView::SelectRows); //select full row
    // change selected row color
    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Highlight, Qt::gray);
    setPalette(*palette);
    for (int i = 0; i < rowCount(); i++) {
        for (int n = 0; n < COL_Total; n++) {
            setItem(i, n, new QTableWidgetItem());
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

    asmLines_[0u].make_list(2);
    asmLines_[0u][COL_addrline].make_uint64(751);
    asmLines_[0u][COL_code].make_string("Some piece of code");

    asmLines_[1].make_list(COL_Total);
    asmLines_[1][COL_addrline].make_uint64(0x0000000000001000);
    asmLines_[1][COL_code].make_uint64(0x94503213);
    asmLines_[1][COL_label].make_string("");
    asmLines_[1][COL_mnemonic].make_string("add     r1,r2,r4");
    asmLines_[1][COL_comment].make_string("'; test");
    outLines();

    selectRow(0);
    //hideRow(0);

    connect(this, SIGNAL(signalHandleResponse()),
            this, SLOT(slotHandleResponse()));
}

void AsmArea::slotPostInit(AttributeType *cfg) {
    AttributeType src_list;
    RISCV_get_services_with_iface(IFACE_SOURCE_CODE, &src_list);
    if (!src_list.is_list() || src_list.size() == 0) {
        return;
    }
    IService *iserv = static_cast<IService *>(src_list[0u].to_iface());
    isrc_ = static_cast<ISourceCode *>(iserv->getInterface(IFACE_SOURCE_CODE));
}

void AsmArea::slotUpdateByTimer() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                            &cmdRegs_, true);
}

void AsmArea::slotHandleResponse() {
    outLines();
}

bool AsmArea::isNeedUpdate() {
    if (npc_ >= addrStart_ && npc_ < (addrStart_ + addrSize_/2)) {
        return false;
    }
    return true;
}

void AsmArea::handleResponse(AttributeType *req, AttributeType *resp) {
    if ((*req).is_equal(cmdRegs_.to_string())) {
        char tstr[128];
        npc_ = resp->to_uint64();
        if (!isNeedUpdate()) {
            emit signalHandleResponse();
            return;
        }

        addrStart_ = npc_;
        addrSize_ = 4 * rowCount();
        RISCV_sprintf(tstr, sizeof(tstr), "read 0x%08" RV_PRI64 "x %d",
                                            addrStart_, addrSize_);
        cmdReadMem_.make_string(tstr);
        igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                                &cmdReadMem_, true);

    } else if (resp->is_data()) {
        data2lines(addrStart_, *resp, asmLines_);
        emit signalHandleResponse();
    } 
}

void AsmArea::outLines() {
    for (unsigned i = 0; i < asmLines_.size(); i++) {
        AttributeType &asmLine = asmLines_[i];
        if (!asmLine.is_list()) {
            continue;
        }
        outLine(static_cast<int>(i), asmLines_[i]);

        if (npc_ == asmLines_[i][COL_addrline].to_uint64()) {
            selectRow(i);
        }
    }
}

void AsmArea::outLine(int idx, AttributeType &line) {
    char stmp[256];
    QTableWidgetItem *pw;
    if (idx >= rowCount()) {
        return;
    }
    if (!line.is_list()) {
        return;
    }
    
    if (line.size() == 2) {
        pw = item(idx, COL_addrline);
        RISCV_sprintf(stmp, sizeof(stmp), "%d", line[COL_addrline].to_int());
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
        RISCV_sprintf(stmp, sizeof(stmp), 
                      "%016" RV_PRI64 "x", line[COL_addrline].to_uint64());
        pw->setText(QString(stmp));
        pw->setTextColor(QColor(Qt::black));

        pw = item(idx, COL_code);
        RISCV_sprintf(stmp, sizeof(stmp), "%08x", line[COL_code].to_int());
        
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

void AsmArea::data2lines(uint64_t npc, AttributeType &data,
                                       AttributeType &lines) {
    uint64_t code;
    unsigned off = 0;
    if (rowCount() != static_cast<int>(lines.size())) {
        return;
    }
    for (int i = 0; i < rowCount(); i++) {
        AttributeType &line = lines[i];
        if (!line.is_list())  {
            line.make_list(COL_Total);
        }
        if (line.size() < COL_Total) {
            setSpan(i, COL_code, 1, 1);
            line.make_list(COL_Total);
        }
        line[COL_addrline].make_uint64(npc + off);

        code = 0;
        for (unsigned n = 0; n < 4; n++) {
            code = (code << 8) | data(off + (3 - n));
        }
        line[COL_code].make_uint64(code);
        line[COL_label].make_string("");
        if (isrc_) {
            off += isrc_->disasm(npc, &data, off,
                                 &line[COL_mnemonic],
                                 &line[COL_comment]);
        } else {
            line[COL_mnemonic].make_string("no disassembler");
            line[COL_comment].make_string("");
            off += 4;
        }
    }
}

}  // namespace debugger
