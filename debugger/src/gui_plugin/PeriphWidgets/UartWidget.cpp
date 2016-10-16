#include "UartWidget.h"
#include "moc_UartWidget.h"

#include <QtWidgets/QHBoxLayout>
#include <memory>

namespace debugger {

UartWidget::UartWidget(IGui *igui, QWidget *parent) 
    : UnclosableWidget(parent) {

    QHBoxLayout *layout = new QHBoxLayout;
    editor_ = new UartEditor(igui, this);
    layout->addWidget(editor_);
    layout->setMargin(0);
    setLayout(layout);
    
    connect(this, SIGNAL(signalPostInit(AttributeType *)),
            editor_, SLOT(slotPostInit(AttributeType *)));
}

void UartWidget::slotPostInit(AttributeType *cfg) {
    emit signalPostInit(cfg);
}

}  // namespace debugger
