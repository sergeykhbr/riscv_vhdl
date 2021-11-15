/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Code Coverage detailed information table
 */

#include "CoverageTable.h"
#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

namespace debugger {

CoverageTable::CoverageTable(QWidget *parent) : QTableWidget(parent) {
    pwidget_ = static_cast<CodeCoverageWidget *>(parent);
    hideLineIdx_ = 0;
    selRowIdx = -1;

    clear();
    QFont font("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    setContentsMargins(QMargins(0, 0, 0, 0));
    QFontMetrics fm(font);
    setMinimumWidth(50 + fm.horizontalAdvance(tr(
    "   0x112233  0x112233   very_long_symbol_name + 00h")));
    lineHeight_ = fm.height() + 4;
    visibleLinesTotal_ = 0;

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
        QString("Start Addr.;End Addr.;Information").split(";"));
    setColumnWidth(0, 10 + fm.horizontalAdvance(tr(" Start Addr. ")));
    setColumnWidth(1, 10 + fm.horizontalAdvance(tr(" End Addr.   ")));
    setColumnWidth(2, 10 + fm.horizontalAdvance(tr("very_long+symbol_name + 00h")));
}

void CoverageTable::slotDatailedInfoUpdate() {
    adjustRowCount();
    clearSpans();

    AttributeType *info = pwidget_->getpDetailedInfo();
    for (unsigned i = 0; i < info->size(); i++) {
        AttributeType &line = (*info)[i];
        if (!line.is_list()) {
            continue;
        }
        outLine(static_cast<int>(i), line);
    }
}

void CoverageTable::adjustRowCount() {
    int need_cnt = static_cast<int>(pwidget_->getpDetailedInfo()->size());
    int row_cnt = rowCount();
    if (row_cnt < need_cnt) {
        Qt::ItemFlags fl;
        setRowCount(need_cnt);
        for (int i = row_cnt; i < need_cnt; i++) {
            for (int n = 0; n < COL_Total; n++) {
                setItem(i, n, new QTableWidgetItem());
                fl = item(i, n)->flags();
                fl &= ~Qt::ItemIsEditable;
                fl &= ~Qt::ItemIsSelectable;
                item(i, n)->setFlags(fl);
            }
            setRowHeight(i, lineHeight_);
        }
    } 
    if (hideLineIdx_ < need_cnt) {
        for (int i = hideLineIdx_; i < need_cnt; i++) {
            showRow(i);
        }
    }
    hideLineIdx_ = need_cnt;
    for (int i = hideLineIdx_; i < rowCount(); i++) {
        hideRow(i);
    }
}

void CoverageTable::outLine(int idx, AttributeType &line) {
    QTableWidgetItem *pw;
    Qt::GlobalColor bkgclr;
    Qt::GlobalColor txtclr;
    if (line[0u].to_bool()) {
        txtclr = Qt::white;
        bkgclr = Qt::darkGreen;
    } else {
        txtclr = Qt::black;
        bkgclr = Qt::white;
    }
    uint64_t addr = line[1].to_uint64();
    uint64_t sz = line[2].to_uint64();

    pw = item(idx, COL_address);
    pw->setText(QString("%1").arg(addr, 6, 16, QChar('0')));
    pw->setForeground(QColor(QColor(txtclr)));
    pw->setBackground(QColor(bkgclr));

    pw = item(idx, COL_size);
    pw->setForeground(QColor(txtclr));
    pw->setBackground(QColor(bkgclr));
    pw->setText(QString("%1").arg(addr+sz-1, 6, 16, QChar('0')));

    pw = item(idx, COL_info);
    pw->setForeground(QColor(txtclr));
    pw->setBackground(QColor(bkgclr));
    pw->setText(QString(line[3].to_string()));
}

}  // namespace debugger
