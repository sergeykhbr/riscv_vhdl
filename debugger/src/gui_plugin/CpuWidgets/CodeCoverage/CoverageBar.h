/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Code Coverage Bar widgets.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QGridLayout>
#include <QtGui/QAction>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtCore/QEvent>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtGui/qevent.h>
#include "CodeCoverageWidget.h"

namespace debugger {

class CoverageProgressBar : public QWidget {
    Q_OBJECT
 public:
    explicit CoverageProgressBar(QWidget *parent);

 protected slots:
    void slotDatailedInfoUpdate();

 protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *ev);

 private:
    void renderAll();

 private:
    CodeCoverageWidget *pwidget_;
    QPixmap pixmap;
    QRect rectMargined;
    uint64_t totalBytes_;
    uint64_t totalPixels_;
    bool needRedraw_;
};

}  // namespace debugger
