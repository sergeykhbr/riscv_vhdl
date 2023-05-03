/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include "iclass.h"
#include "iservice.h"
#include "ihap.h"
#include "coreservices/ithread.h"
#include "coreservices/ijtag.h"
#include "coreservices/icmdexec.h"
#include "generic/tcpclient.h"
#include "../remote/gdbcmd.h"
#include "../exec/cmd/cmd_init.h"
#include "../exec/cmd/cmd_reset.h"
#include "../exec/cmd/cmd_resume.h"
#include "../exec/cmd/cmd_halt.h"
#include "../exec/cmd/cmd_status.h"
#include "../exec/cmd/cmd_reg.h"
#include "../exec/cmd/cmd_read.h"
#include "../exec/cmd/cmd_write.h"
#include "../exec/cmd/cmd_exit.h"
//#include "cmd/cmd_loadelf.h"
//#include "cmd/cmd_loadh86.h"
//#include "cmd/cmd_loadsrec.h"
//#include "cmd/cmd_log.h"
//#include "cmd/cmd_memdump.h"
//#include "cmd/cmd_cpi.h"
//#include "cmd/cmd_loadbin.h"
//#include "cmd/cmd_elf2raw.h"
//#include "cmd/cmd_cpucontext.h"
#include <string>

namespace debugger {

class OpenOcdWrapper : public TcpClient,
                       public IJtag {
 public:
    explicit OpenOcdWrapper(const char *name);
    virtual ~OpenOcdWrapper();

    /** IService interface */
    virtual void postinitService() override;
    virtual void predeleteService() override;

    /** TcpClient */
    virtual int processRxBuffer(const char *buf, int sz);
    virtual int sendData() override;

    /** IJtag interface */
    virtual uint64_t scan(uint32_t ir, uint64_t dr, int drlen);
    virtual void resetAsync();
    virtual void resetSync();
    virtual uint32_t scanIdCode();
    virtual IJtag::DtmcsType scanDtmcs();
    virtual uint32_t scanDmi(uint32_t addr, uint32_t data, IJtag::EDmiOperation op);

    virtual bool isGdbMode() { return gdbMode_.to_bool(); }

 protected:
    /** TcpClient generic methods */
    virtual void afterThreadStarted() override;


 private:
    class ExternalProcessThread : public IService,
                                  public IThread {
     public:
        ExternalProcessThread(IService *parent,
                              const char *name,
                              const char *path,
                              const char *script)
            : IService(name) {
            registerInterface(static_cast<IThread *>(this));
            AttributeType t1;
            RISCV_generate_name(&t1);
            RISCV_event_create(&eventLoopStarted_, t1.to_string());
            path_.make_string(path);
            script_.make_string(script);
            retcode_ = 0;
        }
        virtual ~ExternalProcessThread() {
            RISCV_event_close(&eventLoopStarted_);
        }
        virtual void waitToStart() { RISCV_event_wait(&eventLoopStarted_); }
        virtual int getRetCode() { return retcode_; }

     protected:
        virtual void busyLoop();
     private:
        AttributeType path_;
        AttributeType script_;
        event_def eventLoopStarted_;
        int retcode_;
    };


 private:
    AttributeType isEnable_;
    AttributeType cmdexec_;
    AttributeType pollingMs_;
    AttributeType openOcdPath_;
    AttributeType openOcdScript_;
    AttributeType gdbMode_;
 
    ICmdExecutor *icmdexec_;
    CmdInit cmdInit_;
    CmdReset cmdReset_;
    CmdResume cmdResume_;
    CmdHalt cmdHalt_;
    CmdStatus cmdStatus_;
    CmdExit cmdExit_;

    event_def config_done_;
    event_def eventJtagScanEnd_;
    ExternalProcessThread *openocd_;

    bool connectionDone_;
    enum EMsgParserState {
        MsgIdle,
        MsgData,
        MsgCrcHigh,
        MsgCrcLow,
    } emsgstate_;
    char msgbuf_[1 << 10];
    int msgcnt_;

    GdbCommandGeneric gdbReq_;
    GdbCommandGeneric gdbResp_;

    static const int IRLEN = 5;
    static const int ABITS = 7;     // should be checked in dtmconctrol register

    IJtag::ETapState bbstate_;
    struct scan_request_type {
        bool valid;
        uint32_t ir;
        uint64_t dr;
        int drlen;
    } scanreq_;

    int ircnt_;
    int drcnt_;
    uint32_t ir_;
    uint64_t dr_;
};

DECLARE_CLASS(OpenOcdWrapper)

}  // namespace debugger
