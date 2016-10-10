/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Halt simulation.
 */

#ifndef __DEBUGGER_CMD_HALT_H__
#define __DEBUGGER_CMD_HALT_H__

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdHalt : public ICommand  {
public:
    explicit CmdHalt(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual bool exec(AttributeType *args, AttributeType *res);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_HALT_H__
