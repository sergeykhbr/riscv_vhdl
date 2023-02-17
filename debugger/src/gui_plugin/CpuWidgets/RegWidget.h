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

#include <QtWidgets/QWidget>
#include <QtWidgets/QLineEdit>

namespace debugger {

class RegWidget : public QLineEdit {
    Q_OBJECT
 public:
    RegWidget(const char *name, int bytes, QWidget *parent);

 signals:
    void signalChanged(const char *cmd);
 protected slots:
    void slotHandleResponse(AttributeType *resp);
    void slotEditingFinished();

 protected:
    virtual void outputValue(AttributeType &val);

 protected:
    AttributeType regName_;
    AttributeType cmdRead_;
    AttributeType cmdWrite_;
    char fmtValue_[64];
    QString name_;
    uint64_t value_;
    uint64_t respValue_;
};

}  // namespace debugger
