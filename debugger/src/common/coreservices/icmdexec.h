/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Command executer's interface.
 */

#ifndef __DEBUGGER_ICMDEXEC_H__
#define __DEBUGGER_ICMDEXEC_H__

#include "iface.h"
#include "attribute.h"
#include "icommand.h"

namespace debugger {

static const char *IFACE_CMD_EXECUTOR = "ICmdExecutor";

class ICmdExecutor : public IFace {
public:
    ICmdExecutor() : IFace(IFACE_CMD_EXECUTOR) {}

    /** Register command with ICommand interface */
    virtual void registerCommand(ICommand *icmd) =0;
    virtual void unregisterCommand(ICommand *icmd) =0;

    /** Execute string as a command */
    virtual void exec(const char *line, AttributeType *res, bool silent) =0;

    /** Get list of supported comands starting with substring 'substr' */
    virtual void commands(const char *substr, AttributeType *res) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ICMDEXEC_H__
