#include "RegWidget.h"
#include "moc_RegWidget.h"

#include <memory>
#include <QtWidgets/qlineedit.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>

namespace debugger {

RegWidget::RegWidget(const char *name, QWidget *parent) : QWidget(parent) {
    name_ = name;
    value_ = 0;

    QFont font = QFont("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    font.setFixedPitch(true);
    setFont(font);


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
    label->setText(tr(name_));
    label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    pLayout->addWidget(label);

    QLineEdit *edit = new QLineEdit(this);
    pLayout->addWidget(edit);
    QString def = QString("0x%1").arg(0xfeedfaceull, 16, 16, QChar('0'));
    edit->setText(def);
    edit->setMaximumWidth(135);
    // Line Length: sign plus/minus, '0x' and 16 digits
    edit->setMaxLength(19);
    //edit->setInputMask(tr("#NHHHHHHHHHHHHHHHH"));
}

void RegWidget::slotPollingUpdate(const char *name, uint64_t val) {
    if (strcmp(name_, name) == 0) {
        value_ = val;
    }
}

}  // namespace debugger
