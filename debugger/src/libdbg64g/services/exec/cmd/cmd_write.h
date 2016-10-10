/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Write memory.
 */

#ifndef __DEBUGGER_CMD_WRITE_H__
#define __DEBUGGER_CMD_WRITE_H__

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdWrite : public ICommand  {
public:
    explicit CmdWrite(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual bool exec(AttributeType *args, AttributeType *res);

private:
    AttributeType wrData_;
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_WRITE_H__
