/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Stack Trace main area.
 */

#include "coreservices/ielfreader.h"
#include "StackTraceArea.h"
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

namespace debugger {

StackTraceArea::StackTraceArea(IGui *gui, QWidget *parent)
    : QTableWidget(parent) {
    igui_ = gui;

    clear();
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    setContentsMargins(QMargins(0, 0, 0, 0));
    QFontMetrics fm(font);
    setMinimumWidth(50 + fm.horizontalAdvance(tr(
    "0x0001040 (some_symbol_name+0x40)  0x0001040 (some_symbol_name+0x40)")));
    lineHeight_ = fm.height() + 4;
    hideLineIdx_ = 0;
    symbolList_.make_nil();

    setColumnCount(COL_Total);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setListSize(1);
    
    verticalHeader()->setVisible(false);        // remove row indexes
    setShowGrid(false);                         // remove borders
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows); //select full row

    // change selected row color
    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Highlight, Qt::gray);
    setPalette(*palette);

    setHorizontalHeaderLabels(
        QString("call;at address").split(";"));
    setColumnWidth(COL_at_addr,
        10 + fm.horizontalAdvance(tr("0x0001040 (some_symbol_name+0x40)")));
    setColumnWidth(COL_call_addr,
        10 + fm.horizontalAdvance(tr("0x0001040 (some_symbol_name+0x40)")));

    connect(this, SIGNAL(signalHandleResponse()),
            this, SLOT(slotHandleResponse()));

    connect(this, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(slotCellDoubleClicked(int, int)));

    requested_ = false;
}

StackTraceArea::~StackTraceArea() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void StackTraceArea::slotUpdateByTimer() {
    if (requested_) {
        return;
    }
    requested_ = true;
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           "stack", &symbolList_, true);
}

void StackTraceArea::setListSize(int sz) {
    if (sz > rowCount()) {
        for (int i = hideLineIdx_; i < rowCount(); i++) {
            showRow(i);
        }
        hideLineIdx_ = sz;

        Qt::ItemFlags fl;
        int rowcnt_cur = rowCount();
        setRowCount(sz);
        for (int i = rowcnt_cur; i < rowCount(); i++) {
            for (int n = 0; n < COL_Total; n++) {
                setItem(i, n, new QTableWidgetItem());
                fl = item(i, n)->flags();
                fl &= ~Qt::ItemIsEditable;
                //fl &= ~Qt::ItemIsSelectable;
                item(i, n)->setFlags(fl);
            }
            setRowHeight(i, lineHeight_);
        }
    } else {
        for (int i = hideLineIdx_; i < sz; i++) {
            showRow(i);
        }
        hideLineIdx_ = sz;
        for (int i = sz; i < rowCount(); i++) {
            hideRow(i);
        }
    }
}

void StackTraceArea::handleResponse(const char *cmd) {
    if (strstr(cmd, "stack") == 0) {
        return;
    }
    emit signalHandleResponse();
}

void StackTraceArea::slotHandleResponse() {
    if (!symbolList_.is_list()) {
        return;
    }
    QTableWidgetItem *pw;
    uint64_t addr;
    int list_sz = static_cast<int>(symbolList_.size());
    setListSize(list_sz);
    symbolAddr_.make_list(list_sz);

    for (int i = 0; i < list_sz; i++) {
        AttributeType &symb = symbolList_[i];
        if (!symb.is_list() || symb.size() < 4) {
            continue;
        }
        AttributeType &saddr = symbolAddr_[i];
        saddr.make_list(COL_Total);

        // [from, ['symb_name',symb_offset], to, ['symb_name',symb_offset]]
        addr = symb[2].to_uint64();
        saddr[COL_call_addr].make_uint64(addr);
        pw = item(i, COL_call_addr);
        pw->setText(makeSymbolQString(addr, symb[3]));

        addr = symb[0u].to_uint64();
        saddr[COL_at_addr].make_uint64(addr);
        pw = item(i, COL_at_addr);
        pw->setText(makeSymbolQString(addr, symb[1]));
    }
    symbolList_.attr_free();
    symbolList_.make_nil();
    RISCV_memory_barrier();
    requested_ = false;
}

QString StackTraceArea::makeSymbolQString(uint64_t addr, AttributeType &info) {
    QString ret = QString("%1 ").arg(addr, 8, 16, QChar('0'));
    if (!info.is_list() || info.size() != 2) {
        return ret;
    }
    if (!info[0u].is_string() || info[0u].size() == 0) {
        return ret;
    }
    ret += "(";
    ret += QString(tr(info[0u].to_string()));
    uint64_t offset = info[1].to_uint64();
    if (offset) {
        ret += QString("+%1h").arg(offset, 0, 16);
    }
    ret += ")";
    return ret;
}

void StackTraceArea::slotCellDoubleClicked(int row, int column) {
    uint64_t addr;
    if (row >= static_cast<int>(symbolAddr_.size())) {
        return;
    }
    addr = symbolAddr_[row][column].to_uint64();

    emit signalShowFunction(addr, 0);
}

}  // namespace debugger
