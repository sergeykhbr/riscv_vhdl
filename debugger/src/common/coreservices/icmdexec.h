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

namespace debugger {

static const char *IFACE_CMD_EXECUTOR = "ICmdExecutor";

class ICmdExecutor : public IFace {
public:
    ICmdExecutor() : IFace(IFACE_CMD_EXECUTOR) {}

    virtual bool exec(const char *line, AttributeType *res, bool silent) =0;
    virtual void registerRawListener(IFace *iface) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ICMDEXEC_H__
