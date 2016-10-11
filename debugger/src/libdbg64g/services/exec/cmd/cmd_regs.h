/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read only registers values.
 */

#ifndef __DEBUGGER_CMD_REGS_H__
#define __DEBUGGER_CMD_REGS_H__

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdRegs : public ICommand  {
public:
    explicit CmdRegs(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
    virtual void convert_to_str(AttributeType *lst, AttributeType *out);
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_REGS_H__
