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
 * @brief      Memory Editor area.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QPlainTextEdit>

namespace debugger {

class MemArea : public QPlainTextEdit,
                public IGuiCmdHandler {
    Q_OBJECT
 public:
    MemArea(IGui *gui, QWidget *parent, uint64_t addr, uint64_t sz);
    virtual ~MemArea();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

 signals:
    void signalUpdateData();

 public slots:
    void slotAddressChanged(AttributeType *cmd);
    void slotUpdateByTimer();
    void slotUpdateData();

 private:
    void to_string(uint64_t addr, unsigned bytes, AttributeType *out);

 private:
    AttributeType cmdRead_;
    AttributeType respRead_;
    QString name_;
    IGui *igui_;

    AttributeType data_;
    AttributeType tmpBuf_;
    AttributeType dataText_;

    uint64_t reqAddr_;
    unsigned reqBytes_;
    uint64_t reqAddrZ_;
    unsigned reqBytesZ_;
    bool waitingResponse_;
};

}  // namespace debugger
