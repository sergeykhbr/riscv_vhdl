/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Command Executor declaration.
 */

#ifndef __DEBUGGER_CMDEXECUTOR_H__
#define __DEBUGGER_CMDEXECUTOR_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/icmdexec.h"
#include "coreservices/itap.h"
#include "coreservices/iautocomplete.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"
#include <string>
#include <stdarg.h>

namespace debugger {

class CmdExecutor : public IService,
                    public ICmdExecutor {
public:
    explicit CmdExecutor(const char *name);
    virtual ~CmdExecutor();

    /** IService interface */
    virtual void postinitService();

    /** ICmdExecutor */
    virtual void exec(const char *line, AttributeType *res, bool silent);

private:
    void processSimple(AttributeType *cmd, AttributeType *res);
    void processScript(AttributeType *cmd, AttributeType *res);
    void splitLine(char *str, AttributeType *listArgs);

    int outf(const char *fmt, ...);
    bool cmdIsError(AttributeType *res);
    ICommand *getICommand(AttributeType *args);
    ICommand *getICommand(const char *name);

private:
    AttributeType tap_;
    AttributeType socInfo_;
    AttributeType cmds_;

    ITap *itap_;
    ISocInfo *info_;

    mutex_def mutexExec_;

    char cmdbuf_[4096];
    char *outbuf_;
    int outbuf_size_;
    int outbuf_cnt_;
    uint8_t *tmpbuf_;
    int tmpbuf_size_;
};

DECLARE_CLASS(CmdExecutor)

}  // namespace debugger

#endif  // __DEBUGGER_CMDEXECUTOR_H__
