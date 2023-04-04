/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_CMDEXECUTOR_H__
#define __DEBUGGER_CMDEXECUTOR_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/icmdexec.h"
#include "coreservices/iautocomplete.h"
#include "coreservices/icommand.h"
#include "coreservices/imemop.h"
#include "coreservices/ijtag.h"
#include "generic/tcpclient.h"
#include <string>
#include <stdarg.h>

namespace debugger {

class CmdExecutor : public TcpClient,
                    public ICmdExecutor {
 public:
    explicit CmdExecutor(const char *name);
    virtual ~CmdExecutor();

    /** IService interface */
    virtual void postinitService() override;

    /** ICmdExecutor */
    virtual void registerCommand(ICommand *icmd);
    virtual void unregisterCommand(ICommand *icmd);
    virtual void exec(const char *line, AttributeType *res, bool silent);
    virtual void commands(const char *substr, AttributeType *res);

 protected:
    /** TcpClient */
    virtual int processRxBuffer(const char *ibuf, int ilen) { return 0; }

 private:
    void processSimple(AttributeType *cmd, AttributeType *res);
    void processScript(AttributeType *cmd, AttributeType *res);
    void splitLine(char *str, AttributeType *listArgs);

    int outf(const char *fmt, ...);
    bool cmdIsError(AttributeType *res);
    int getICommand(AttributeType *args, ICommand **pcmd);

 private:
    AttributeType bus_;
    AttributeType jtag_;
    AttributeType cmds_;

    IMemoryOperation *ibus_;
    IJtag *ijtag_;

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
