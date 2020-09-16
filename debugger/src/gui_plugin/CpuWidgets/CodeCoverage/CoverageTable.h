/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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
 *
 * @brief      Code Coverage detailed information table.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "iservice.h"
#include "coreservices/isrccode.h"
#include "CodeCoverageWidget.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableWidget>
#include <QtGui/QResizeEvent>

namespace debugger {

class CoverageTable : public QTableWidget {
    Q_OBJECT
 public:
    explicit CoverageTable(QWidget *parent);

 public slots:
    void slotDatailedInfoUpdate();

 private:
    void adjustRowCount();
    void outLine(int idx, AttributeType &data);

 private:
    enum EColumnNames {
        COL_address,
        COL_size,
        COL_info,
        COL_Total
    };

    CodeCoverageWidget *pwidget_;

    int selRowIdx;
    int hideLineIdx_;
    int lineHeight_;
    int visibleLinesTotal_;
};

}  // namespace debugger
