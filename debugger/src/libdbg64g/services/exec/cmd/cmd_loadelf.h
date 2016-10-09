/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Elf-file loader command.
 */

#ifndef __DEBUGGER_CMD_HELP_H__
#define __DEBUGGER_CMD_HELP_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdLoadElf : public ICommand  {
public:
    explicit CmdLoadElf(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual bool exec(AttributeType *args, AttributeType *res);
    virtual bool format(AttributeType *res, AttributeType *out);

private:
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_REGS_H__
