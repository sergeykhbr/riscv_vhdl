/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      User's command interface.
 */

#ifndef __DEBUGGER_ICOMMAND_H__
#define __DEBUGGER_ICOMMAND_H__

#include "iface.h"
#include "attribute.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"


namespace debugger {

static const char *IFACE_COMMAND = "ICommand";

static const bool CMD_VALID     = true;
static const bool CMD_INVALID   = false;

class ICommand : public IFace {
public:
    ICommand(const char *name, ITap *tap, ISocInfo *info) 
        : IFace(IFACE_COMMAND) {
        cmdName_.make_string(name);
        tap_ = tap;
        info_ = info;
    }
    virtual ~ICommand() {}

    virtual const char *cmdName() { return cmdName_.to_string(); }
    virtual const char *briefDescr() { return briefDescr_.to_string(); }
    virtual const char *detailedDescr() { return detailedDescr_.to_string(); }

    virtual bool isValid(AttributeType *args) =0;
    virtual void exec(AttributeType *args, AttributeType *res) =0;

    virtual void generateError(AttributeType *res, const char *descr) {
        res->make_list(3);
        (*res)[0u].make_string("ERROR");
        (*res)[1].make_string(cmdName_.to_string());
        (*res)[2].make_string(descr);
    }

protected:
    AttributeType cmdName_;
    AttributeType briefDescr_;
    AttributeType detailedDescr_;
    ITap *tap_;
    ISocInfo *info_;
};

}  // namespace debugger

#endif  // __DEBUGGER_ICOMMAND_H__
