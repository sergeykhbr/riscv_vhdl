/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Browse symbols command.
 */

#ifndef __DEBUGGER_CMD_SYMB_H__
#define __DEBUGGER_CMD_SYMB_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdSymb : public ICommand  {
public:
    explicit CmdSymb(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_SYMB_H__
