/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Stack Trace main area.
 */

#include "coreservices/ielfreader.h"
#include "StackTraceArea.h"
#include "moc_StackTraceArea.h"

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
    setMinimumWidth(50 + fm.width(tr(
    "some_test_function  (void ())       P:aabbccdd--aabbffff")));
    lineHeight_ = fm.height() + 4;
    hideLineIdx_ = 0;

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
        QString("address;symbol").split(";"));
    setColumnWidth(COL_address, 10 + fm.width(tr("0x0000000000001040 ")));
    setColumnWidth(COL_symbol, 10 + fm.width(tr("some_symbol_name      ")));
}

StackTraceArea::~StackTraceArea() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
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

void StackTraceArea::handleResponse(AttributeType *req,
                                    AttributeType *resp) {
    if (strstr(req->to_string(), "stack") == 0) {
        return;
    }
    symbolList_ = *resp;
    emit signalHandleResponse();
}

void StackTraceArea::slotHandleResponse() {
    if (!symbolList_.is_list()) {
        return;
    }
    QTableWidgetItem *pw;
    Qt::ItemFlags fl;
    char tstr[256];
    uint64_t addr;
    int list_sz = static_cast<int>(symbolList_.size());
    setListSize(list_sz);

    for (int i = 0; i < list_sz; i++) {
        AttributeType &symb = symbolList_[i];

        pw = item(i, COL_symbol);
        pw->setText(QString(symb[Symbol_Name].to_string()));

        pw = item(i, COL_address);
        addr = symb[Symbol_Addr].to_uint64();
        RISCV_sprintf(tstr, sizeof(tstr),
                    "%08" RV_PRI64 "x--%08" RV_PRI64 "x",
                    addr, addr + symb[Symbol_Size].to_uint64());
        pw->setText(QString(tstr));
    }
}

}  // namespace debugger
