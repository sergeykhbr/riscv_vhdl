/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Read CPU dport registers (step counter and clock 
 *             counter) to compute CPI in run-time.
 */

#ifndef __DEBUGGER_CMD_CPI_H__
#define __DEBUGGER_CMD_CPI_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdCpi : public ICommand  {
public:
    explicit CmdCpi(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
    uint64_t stepCnt_z;
    uint64_t clockCnt_z;
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_CPI_H__
