/**
 * @file
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Binary-file loader command.
 */

#ifndef __DEBUGGER_SERVICES_EXEC_CMD_LOADBIN_H__
#define __DEBUGGER_SERVICES_EXEC_CMD_LOADBIN_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdLoadBin : public ICommand  {
 public:
    explicit CmdLoadBin(ITap *tap, ISocInfo *info);

    /** ICommand interface */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

 private:
};

}  // namespace debugger

#endif  // __DEBUGGER_SERVICES_EXEC_CMD_LOADBIN_H__
