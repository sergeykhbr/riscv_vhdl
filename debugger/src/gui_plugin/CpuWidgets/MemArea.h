/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Editor area.
 */

#pragma once

#include "api_core.h"   // MUST BE BEFORE QtWidgets.h or any other Qt header.
#include "attribute.h"
#include "igui.h"
#include "coreservices/isocinfo.h"

#include <QtWidgets/QWidget>
#include <QtWidgets/QPlainTextEdit>

namespace debugger {

class MemArea : public QPlainTextEdit,
                public IGuiCmdHandler {
    Q_OBJECT
public:
    explicit MemArea(IGui *gui, QWidget *parent);

    /** IGuiCmdHandler */
    virtual void handleResponse(AttributeType *req, AttributeType *resp);

signals:
    void signalUpdateData();

public slots:
    void slotAddressChanged(AttributeType *cmd);
    void slotUpdateByTimer();
    void slotUpdateData();

private:
    void to_string(uint64_t addr, uint64_t bytes, AttributeType *out);

private:
    AttributeType cmdRead_;
    QString name_;
    IGui *igui_;

    AttributeType data_;
    AttributeType tmpBuf_;
    AttributeType dataText_;

    uint64_t reqAddr_;
    unsigned reqBytes_;
};

}  // namespace debugger
