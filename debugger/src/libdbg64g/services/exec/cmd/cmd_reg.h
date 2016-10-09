/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read/write register value.
 */

#ifndef __DEBUGGER_CMD_REG_H__
#define __DEBUGGER_CMD_REG_H__

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdReg : public ICommand  {
public:
    explicit CmdReg(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual bool exec(AttributeType *args, AttributeType *res);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_REG_H__
