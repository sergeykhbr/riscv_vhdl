#include "UartWidget.h"
#if!defined(CMAKE_ENABLED)
#include "moc_UartWidget.h"
#endif
#include <QtWidgets/QHBoxLayout>
#include <memory>

namespace debugger {

UartWidget::UartWidget(IGui *igui, QWidget *parent) 
    : QWidget(parent) {

    QHBoxLayout *layout = new QHBoxLayout;
    editor_ = new UartEditor(igui, this);
    layout->addWidget(editor_);
    layout->setMargin(0);
    setLayout(layout);
}

}  // namespace debugger
