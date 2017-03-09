/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Disassembler viewer area.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "iservice.h"
#include "coreservices/isocinfo.h"
#include "coreservices/isrccode.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableWidget>
#include <QtGui/QResizeEvent>

namespace debugger {

class AsmArea : public QTableWidget,
                public IGuiCmdHandler {
    Q_OBJECT
public:
    explicit AsmArea(IGui *gui, QWidget *parent, uint64_t fixaddr);
    virtual ~AsmArea();

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

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
    AttributeType cmdRegs_;
    AttributeType asmLines_;
    AttributeType asmLinesOut_;
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
};

}  // namespace debugger
