/*
 *  Copyright 2021 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "MdiAreaWidget.h"

namespace debugger {

MdiAreaWidget::MdiAreaWidget(AttributeType &cfg, QWidget *parent)
    : QMdiArea(parent) {
    Config_ = cfg;
    if (Config_["Tabbed"].to_bool()) {
        setViewMode(QMdiArea::TabbedView);
    } else {
    }
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
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

