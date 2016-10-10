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
    virtual ~CmdMemDump();

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual bool exec(AttributeType *args, AttributeType *res);
    virtual bool format(AttributeType *args, AttributeType *res, AttributeType *out);

private:
    uint8_t *rdBuf_;
    int rdBufSz_;
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_MEMDUMP_H__
