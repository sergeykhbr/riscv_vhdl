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
#include "coreservices/irawlistener.h"
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
    virtual bool exec(const char *line, AttributeType *res, bool silent);
    virtual void registerRawListener(IFace *iface);

private:
    void processSimple(AttributeType *cmd, AttributeType *res);
    void processScript(AttributeType *cmd, AttributeType *res);
    void splitLine(char *str, AttributeType *listArgs);

    void readCSR(AttributeType *listArgs);
    void writeCSR(AttributeType *listArgs);
    void readMem(AttributeType *listArgs);
    void writeMem(AttributeType *listArgs);
    void memDump(AttributeType *listArgs);

    // Only simulation platform supports these commands (for now):
    void halt(AttributeType *listArgs);
    void run(AttributeType *listArgs);
    void regs(AttributeType *listArgs, AttributeType *res);
    void br(AttributeType *listArgs);

    int outf(const char *fmt, ...);

private:
    AttributeType tap_;
    AttributeType socInfo_;
    AttributeType listeners_;
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
