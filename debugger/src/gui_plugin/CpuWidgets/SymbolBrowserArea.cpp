/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Symbol Browser main area.
 */

#include "coreservices/isrccode.h"
#include "SymbolBrowserArea.h"
#include "moc_SymbolBrowserArea.h"

#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

namespace debugger {

SymbolBrowserArea::SymbolBrowserArea(IGui *gui, QWidget *parent)
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
        QString("symbol;type;address").split(";"));
    setColumnWidth(COL_symbol, 10 + fm.width(tr("some_test_function ")));
    setColumnWidth(COL_type, 10 + fm.width(tr("(void ())      ")));
    setColumnWidth(COL_address, 10 + fm.width(tr("P:0x00001000--0x00001040")));

    connect(this, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(slotCellDoubleClicked(int, int)));

    connect(this, SIGNAL(signalHandleResponse()),
            this, SLOT(slotHandleResponse()));

    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           "symb", &symbolList_, true);
}

SymbolBrowserArea::~SymbolBrowserArea() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void SymbolBrowserArea::setListSize(int sz) {
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

void SymbolBrowserArea::handleResponse(const char *cmd) {
    if (strstr(cmd, "symb") == 0) {
        return;
    }
    emit signalHandleResponse();
}

void SymbolBrowserArea::slotHandleResponse() {
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

        pw = item(i, COL_type);
        if (symb[Symbol_Type].to_uint64() & SYMBOL_TYPE_FUNCTION) {
            pw->setText(tr("function"));
        } else if (symb[Symbol_Type].to_uint64() & SYMBOL_TYPE_DATA) {
            pw->setText(tr("data"));
        } else {
        }

        pw = item(i, COL_address);
        addr = symb[Symbol_Addr].to_uint64();
        RISCV_sprintf(tstr, sizeof(tstr),
                    "%08" RV_PRI64 "x--%08" RV_PRI64 "x",
                    addr, addr + symb[Symbol_Size].to_uint64());
        pw->setText(QString(tstr));
    }
}

void SymbolBrowserArea::slotFilterChanged(const QString &flt) {
    AttributeType tcmd;
    QByteArray flt_buf;
    flt_buf.append("symb " + flt);
    tcmd.make_string(flt_buf.data());
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           tcmd.to_string(), &symbolList_, true);
}

void SymbolBrowserArea::slotCellDoubleClicked(int row, int column) {
    if (row >= static_cast<int>(symbolList_.size())) {
        return;
    }

    AttributeType &symb = symbolList_[row];
    uint64_t addr = symb[Symbol_Addr].to_uint64();
    uint64_t sz = symb[Symbol_Size].to_uint64();

    if (symb[Symbol_Type].to_uint64() & SYMBOL_TYPE_FUNCTION) {
        emit signalShowFunction(addr, sz);
    } else {
        emit signalShowData(addr, sz);
    }
}

}  // namespace debugger
