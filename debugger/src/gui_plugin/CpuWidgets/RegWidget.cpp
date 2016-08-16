#include "RegWidget.h"
#include "moc_RegWidget.h"

#include <memory>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>

namespace debugger {

RegWidget::RegWidget(AttributeType *cfg, QWidget *parent) : QWidget(parent) {
    name_ = QString(tr((*cfg)[0u].to_string()));
    idx_ = (*cfg)[1u].to_uint64();
    value_ = 0;

    while (name_.size() < 3) {
        name_ += tr(" ");
    }

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

void RegWidget::slotRegisterValue(uint64_t idx, uint64_t val) {
    if (idx != idx_) {
        return;
    }
    value_ = val;
    QString def = QString("0x%1").arg(value_, 16, 16, QChar('0'));
    edit_->setText(def);
}

}  // namespace debugger
