/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Run simulation.
 */

#ifndef __DEBUGGER_CMD_RUN_H__
#define __DEBUGGER_CMD_RUN_H__

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdRun : public ICommand  {
public:
    explicit CmdRun(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual bool exec(AttributeType *args, AttributeType *res);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_RUN_H__
