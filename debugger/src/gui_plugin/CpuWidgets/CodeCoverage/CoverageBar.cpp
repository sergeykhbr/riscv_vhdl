/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Code Coverage Bar widgets.
 */

#include "CoverageBar.h"
#include <memory>

namespace debugger {

static int MARGIN = 4;

CoverageProgressBar::CoverageProgressBar(QWidget *parent) : QWidget(parent) {
    pwidget_ = static_cast<CodeCoverageWidget *>(parent);
    QFont font = QFont("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    QFontMetrics fm(font);

    //QGridLayout *gridLayout = new QGridLayout(this);
    //setLayout(gridLayout);

    setMinimumHeight(fm.height() + 8);
    //QLabel *lbl1 = new QLabel("Test:");

    //gridLayout->addWidget(lbl1, 0, 0, Qt::AlignRight);
    needRedraw_ = true;
    totalBytes_ = 0;
    totalPixels_ = 0;
}

void CoverageProgressBar::slotDatailedInfoUpdate() {
    AttributeType *info = pwidget_->getpDetailedInfo();
    totalBytes_ = 0;
    for (unsigned i = 0; i < info->size(); i++) {
        AttributeType &line = (*info)[i];
        if (!line.is_list()) {
            continue;
        }
        totalBytes_ += (line[2].to_uint32() - line[1].to_uint32()) + 1;
    }
    needRedraw_ = true;
}

void CoverageProgressBar::resizeEvent(QResizeEvent *ev) {
    int w = ev->size().width();
    int h = ev->size().height();
    if (w == 0 || h == 0) {
        return;
    }
    QSize pixmapSingleSize = QSize(w, h);
    rectMargined.setTopLeft(QPoint(MARGIN, MARGIN));
    rectMargined.setBottomRight(QPoint(w - MARGIN, h - MARGIN));
    totalPixels_ = static_cast<uint64_t>(rectMargined.width());
    pixmap = QPixmap(pixmapSingleSize);
    needRedraw_ = true;
}

void CoverageProgressBar::paintEvent(QPaintEvent *event) {
    if (needRedraw_) {
        renderAll();
    }

    QPainter p(this);
    QPoint pos(0,0);
    p.drawPixmap(pos, pixmap);
    p.end();
    update();
    needRedraw_ = false;
}

void CoverageProgressBar::renderAll() {
    pixmap.fill(Qt::white);
    QPainter p(&pixmap);

    p.setRenderHint(QPainter::Antialiasing, true);

    p.fillRect(rectMargined, Qt::white);

    AttributeType *info = pwidget_->getpDetailedInfo();
    QRect range(rectMargined.left(), rectMargined.top(),
                0, rectMargined.height());

    uint64_t accBytes = 0;
    uint64_t diff;
    for (unsigned i = 0; i < info->size(); i++) {
        AttributeType &line = (*info)[i];
        if (!line.is_list()) {
            continue;
        }
        diff = line[2].to_uint64() - line[1].to_uint64() + 1;
        if (line[0u].to_bool()) {
            double drate = 0;
            if (totalBytes_) {
                drate = static_cast<double>(accBytes) / totalBytes_;
            }
            drate *= totalPixels_;
            range.setLeft(rectMargined.left() + static_cast<int>(drate + 0.5));

            drate = 0;
            if (totalBytes_) {
                drate = static_cast<double>(accBytes + diff)
                        / totalBytes_;
            }
            drate *= totalPixels_;
            range.setRight(rectMargined.left()
                           + static_cast<int>(drate + 0.5));

            p.fillRect(range, Qt::darkGreen);
        }
        accBytes += diff;
    }

    // Draw Frame above progress bar:
    p.setPen(QPen(QColor(0x48,0x3D,0x8B)));  // Color Dark State Blue: #483D8B
    p.drawRect(rectMargined);

    p.end();
}

}  // namespace debugger
