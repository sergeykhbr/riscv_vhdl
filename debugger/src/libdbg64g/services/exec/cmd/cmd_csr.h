/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read/write CSR register value.
 */

#ifndef __DEBUGGER_CMD_CSR_H__
#define __DEBUGGER_CMD_CSR_H__

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdCsr : public ICommand  {
public:
    explicit CmdCsr(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual bool exec(AttributeType *args, AttributeType *res);
    virtual bool format(AttributeType *args, AttributeType *res, AttributeType *out);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_CSR_H__
