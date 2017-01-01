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

namespace debugger {

class AsmArea : public QTableWidget,
                public IGuiCmdHandler {
    Q_OBJECT
public:
    explicit AsmArea(IGui *gui, QWidget *parent);

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalHandleResponse();

public slots:
    void slotPostInit(AttributeType *cfg);
    void slotUpdateByTimer();
    void slotHandleResponse();

private:
    bool isNeedUpdate();
    void outLines();
    void outLine(int idx, AttributeType &data);
    void data2lines(uint64_t npc, AttributeType &data, AttributeType &lines);

private:
    AttributeType cmdReadMem_;
    AttributeType cmdRegs_;
    AttributeType asmLines_;
    QString name_;
    IGui *igui_;
    ISourceCode *isrc_;

    enum EColumnNames {
        COL_addrline,
        COL_code,
        COL_label,
        COL_mnemonic,
        COL_comment,
        COL_Total
    };

    enum ECmdState {
        CMD_idle,
        CMD_npc,
        CMD_memdata
    };

    AttributeType data_;
    AttributeType tmpBuf_;
    AttributeType dataText_;
    int lineHeight_;
    ECmdState state_;
    uint64_t npc_;
    uint64_t addrStart_;
    uint64_t addrSize_;
};

}  // namespace debugger
