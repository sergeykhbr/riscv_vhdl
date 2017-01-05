/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Add or remove memory breakpoint.
 */

#ifndef __DEBUGGER_CMD_BR_H__
#define __DEBUGGER_CMD_BR_H__

#include "api_core.h"
#include "iservice.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"
#include "coreservices/isrccode.h"

namespace debugger {

class CmdBr : public ICommand  {
public:
    explicit CmdBr(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
    ISourceCode *isrc_;
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_LOG_H__
