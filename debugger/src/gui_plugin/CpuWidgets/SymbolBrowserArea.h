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
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "iservice.h"
#include "coreservices/isrccode.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableWidget>

namespace debugger {

class SymbolBrowserArea : public QTableWidget,
                          public IGuiCmdHandler {
    Q_OBJECT
 public:
    SymbolBrowserArea(IGui *gui, QWidget *parent);
    virtual ~SymbolBrowserArea();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

 public slots:
    void slotCellDoubleClicked(int row, int column);
    void slotFilterChanged(const QString &flt);
    void slotHandleResponse();

 signals:
    void signalHandleResponse();
    void signalShowFunction(uint64_t addr, uint64_t sz);
    void signalShowData(uint64_t addr, uint64_t sz);

 private:
    void setListSize(int sz);

 private:
    enum EColumnNames {
        COL_symbol,
        COL_type,
        COL_address,
        COL_Total
    };

    AttributeType symbolList_;
    AttributeType symbolFilter_;
    IGui *igui_;
    int lineHeight_;
    int hideLineIdx_;
};

}  // namespace debugger
