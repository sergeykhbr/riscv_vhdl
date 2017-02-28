/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read CPU stack trace buffer.
 */

#ifndef __DEBUGGER_CMD_STACK_H__
#define __DEBUGGER_CMD_STACK_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdStack : public ICommand  {
public:
    explicit CmdStack(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_STACK_H__
