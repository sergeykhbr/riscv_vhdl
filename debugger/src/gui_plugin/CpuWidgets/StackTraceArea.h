/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Stack Trace main area.
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

class StackTraceArea : public QTableWidget,
                       public IGuiCmdHandler {
    Q_OBJECT
public:
    explicit StackTraceArea(IGui *gui, QWidget *parent);
    virtual ~StackTraceArea();

    /** IGuiCmdHandler */
    virtual void handleResponse(const char *cmd);

public slots:
    void slotUpdateByTimer();
    void slotHandleResponse();
    void slotCellDoubleClicked(int row, int column);

signals:
    void signalHandleResponse();
    void signalShowFunction(uint64_t addr, uint64_t sz);

private:
    void setListSize(int sz);
    QString makeSymbolQString(uint64_t addr, AttributeType &info);

private:
    enum EColumnNames {
        COL_call_addr,
        COL_at_addr,
        COL_Total
    };

    AttributeType symbolList_;
    AttributeType symbolAddr_;
    bool requested_;
    IGui *igui_;
    int lineHeight_;
    int hideLineIdx_;
};

}  // namespace debugger
