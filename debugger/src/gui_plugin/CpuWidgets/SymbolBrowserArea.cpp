/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Symbol Browser main area.
 */

#include "coreservices/ielfreader.h"
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

    setColumnCount(COL_Total);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setRowCount(1);
    
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
    }

    setHorizontalHeaderLabels(
        QString("symbol;type;address").split(";"));
    setColumnWidth(COL_symbol, 10 + fm.width(tr("some_test_function ")));
    setColumnWidth(COL_type, 10 + fm.width(tr("(void ())      ")));
    setColumnWidth(COL_address, 10 + fm.width(tr("P:0x00001000--0x00001040")));

    connect(this, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(slotCellDoubleClicked(int, int)));

    connect(this, SIGNAL(signalSymbolsUpdated()),
            this, SLOT(slotSymbolsUpdated()));

    AttributeType tcmd("symb");
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           &tcmd, true);
}

SymbolBrowserArea::~SymbolBrowserArea() {
    igui_->removeFromQueue(static_cast<IGuiCmdHandler *>(this));
}

void SymbolBrowserArea::handleResponse(AttributeType *req,
                                       AttributeType *resp) {
    if (strstr(req->to_string(), "symb") == 0) {
        return;
    }
    symbolList_ = *resp;
    emit signalSymbolsUpdated();
}

void SymbolBrowserArea::slotSymbolsUpdated() {
    if (!symbolList_.size() || !symbolList_.is_list()) {
        return;
    }
    QTableWidgetItem *pw;
    Qt::ItemFlags fl;
    char tstr[256];
    uint64_t addr;
    int rowcnt_cur = rowCount();
    
    setRowCount(symbolList_.size());
    for (int i = rowcnt_cur; i < rowCount(); i++) {
        for (int n = 0; n < COL_Total; n++) {
            setItem(i, n, new QTableWidgetItem());
            fl = item(i, n)->flags();
            fl &= ~Qt::ItemIsEditable;
            fl &= ~Qt::ItemIsSelectable;
            item(i, n)->setFlags(fl);
        }
        setRowHeight(i, lineHeight_);
    }

    for (int i = 0; i < rowCount(); i++) {
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

void SymbolBrowserArea::slotFilterApply(AttributeType *flt) {
    char tstr[1024] = "symb";
    AttributeType tcmd;
    if (flt->size() && !flt->is_equal("*")) {
        RISCV_sprintf(tstr, sizeof(tstr), "symb %s", flt->to_string());
    }
    tcmd.make_string(tstr);
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this),
                           &tcmd, true);
}

void SymbolBrowserArea::slotCellDoubleClicked(int row, int column) {
    
}

}  // namespace debugger
