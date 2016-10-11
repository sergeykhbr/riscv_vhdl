/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Dump memory range into file.
 */

#ifndef __DEBUGGER_CMD_MEMDUMP_H__
#define __DEBUGGER_CMD_MEMDUMP_H__

#include "api_core.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdMemDump : public ICommand  {
public:
    explicit CmdMemDump(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_MEMDUMP_H__
