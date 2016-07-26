
#include "MdiAreaWidget.h"
#include "moc_MdiAreaWidget.h"

namespace debugger {

MdiAreaWidget::MdiAreaWidget(AttributeType &cfg, QWidget *parent)
    : QMdiArea(parent) {
    Config_ = cfg;
    if (Config_["Tabbed"].to_bool()) {
        setViewMode(QMdiArea::TabbedView);
    }
    setTabsClosable(true);
    setTabsMovable(true);
}

   
void MdiAreaWidget::slotRemoveView(AttributeType &cfg)
{
    QMdiSubWindow *v = findMdiChild(cfg["Class"].to_string());
    if (v) {
        v->close();
    }
}

QMdiSubWindow *MdiAreaWidget::findMdiChild(const char* name)
{
    const char *check_name;
    QList<QMdiSubWindow *>lst = subWindowList();
    for (int i=0; i<lst.size(); i++) {
        
        check_name = lst[i]->widget()->metaObject()->className();
        if (strcmp(name, check_name) == 0) {
            return lst[i];
        }
    }
    return 0;
}

}  // namespace debugger

