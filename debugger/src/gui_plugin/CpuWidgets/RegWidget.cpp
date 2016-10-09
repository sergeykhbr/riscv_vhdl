#include "RegWidget.h"
#include "moc_RegWidget.h"

#include <memory>
#include <string.h>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>

namespace debugger {

RegWidget::RegWidget(const char *name, IGui *gui, QWidget *parent) 
    : QWidget(parent) {
    igui_ = gui;
    value_ = 0;

    name_ = QString(name);
    while (name_.size() < 3) {
        name_ += tr(" ");
    }

    std::string t1 = "read " + std::string(name);
    cmdRead_.make_string(t1.c_str());

    QFont font = QFont("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);
    QFontMetrics fm(font);

    QHBoxLayout *pLayout = new QHBoxLayout;
    pLayout->setContentsMargins(4, 1, 4, 1);
    setLayout(pLayout);

    QLabel *label = new QLabel(this);
    QSizePolicy labelSizePolicy(QSizePolicy::Preferred, 
                                QSizePolicy::Preferred);
    labelSizePolicy.setHorizontalStretch(0);
    labelSizePolicy.setVerticalStretch(0);
    labelSizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
    label->setSizePolicy(labelSizePolicy);
    label->setText(name_);
    label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    pLayout->addWidget(label);

    edit_ = new QLineEdit(this);
    pLayout->addWidget(edit_);
    QString def = QString("0x%1").arg(0xfeedfaceull, 16, 16, QChar('0'));
    edit_->setText(def);
    edit_->setMaxLength(19);
    edit_->setFixedWidth(fm.width(def) + 8);
    edit_->setFixedHeight(fm.height() + 2);

    setMinimumWidth(edit_->width() + fm.width(name_) + 16);
    setMinimumHeight(edit_->height());
}

void RegWidget::slotUpdateByTimer() {
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), 
                            &cmdRead_, true);
}

void RegWidget::handleResponse(AttributeType *req, AttributeType *resp) {
    value_ = resp->to_uint64();
    QString def = QString("0x%1").arg(value_, 16, 16, QChar('0'));
    edit_->setText(def);
}


}  // namespace debugger
