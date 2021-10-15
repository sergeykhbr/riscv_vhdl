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
 * @brief      Disassembler viewer area.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "iservice.h"
#include "coreservices/isrccode.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableWidget>
#include <QtGui/QResizeEvent>

namespace debugger {

class AsmArea : public QTableWidget,
                public IGuiCmdHandler {
    Q_OBJECT
 public:
    AsmArea(IGui *gui, QWidget *parent, uint64_t fixaddr);
    virtual ~AsmArea();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

 signals:
    void signalNpcChanged();
    void signalAsmListChanged();
    void signalBreakpointsChanged();

 public slots:
    void slotNpcChanged();
    void slotAsmListChanged();
    void slotUpdateByTimer();
    void slotRedrawDisasm();
    void slotCellDoubleClicked(int row, int column);

 protected:
    void resizeEvent(QResizeEvent *ev) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent * ev) Q_DECL_OVERRIDE;

 private:
    bool isNpcTrackEna();
    int getNpcRowIdx();
    void selectNpcRow(int idx);
    void adjustRowCount();
    void outSymbolLine(int idx, AttributeType &data);
    void outAsmLine(int idx, AttributeType &data);
    void addMemBlock(AttributeType &resp, AttributeType &lines);

 private:
    enum EColumnNames {
        COL_addrline,
        COL_code,
        COL_label,
        COL_mnemonic,
        COL_comment,
        COL_Total
    };

    AttributeType cmdReadMem_;
    AttributeType asmLines_;
    AttributeType asmLinesOut_;
    AttributeType reqNpc_;
    AttributeType respNpc_;
    AttributeType respReadMem_;
    AttributeType respBr_;
    QString name_;
    IGui *igui_;

    uint64_t fixaddr_;
    int selRowIdx;
    int hideLineIdx_;
    int lineHeight_;
    mutex_def mutexAsmGaurd_;
    uint64_t npc_;
    int visibleLinesTotal_;
    uint64_t startAddr_;
    uint64_t endAddr_;
    bool waitRegNpc_;
};

}  // namespace debugger
