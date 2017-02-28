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
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

public slots:
    void slotHandleResponse();

signals:
    void signalHandleResponse();

private:
    void setListSize(int sz);

private:
    enum EColumnNames {
        COL_address,
        COL_symbol,
        COL_Total
    };

    AttributeType symbolList_;
    IGui *igui_;
    int lineHeight_;
    int hideLineIdx_;
};

}  // namespace debugger
