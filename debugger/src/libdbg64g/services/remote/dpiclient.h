/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_LIBDBG64G_SERVICES_REMOTE_DPICLIENT_H__
#define __DEBUGGER_LIBDBG64G_SERVICES_REMOTE_DPICLIENT_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ithread.h"
#include "coreservices/idpi.h"
#include "coreservices/icmdexec.h"

namespace debugger {

class CmdDpi : public ICommand  {
 public:
    CmdDpi(IService *parent);

    /** ICommand */
    virtual int isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

 private:
    IService *parent_;
};

class DpiClient : public IService,
                  public IThread,
                  public IDpi {
 public:
    explicit DpiClient(const char *name);
    virtual ~DpiClient();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService() override;

    /** IDpi */
    virtual void axi4_write(uint64_t addr, uint64_t data);
    virtual void axi4_read(uint64_t addr, uint64_t *data);
    virtual bool is_irq();
    virtual int get_irq();

    /** Common methods */

 protected:
    /** IThread interface */
    virtual void busyLoop();

 protected:
    int createServerSocket();
    void closeServerSocket();
    void processRx();
    void processTx();
    void writeTx(const char *buf, unsigned size);
    void syncRequest(const char *buf, unsigned size, AttributeType &resp);

 private:
    AttributeType isEnable_;
    AttributeType cmdexec_;
    AttributeType timeout_;
    AttributeType hostIP_;
    AttributeType hostPort_;
    AttributeType hartBeat_;
    AttributeType response_;

    ICmdExecutor *iexec_;
    CmdDpi cmd_;

    struct sockaddr_in sockaddr_ipv4_;
    socket_def hsock_;

    mutex_def mutex_tx_;
    event_def event_cmd_;
    char rcvbuf[4096];
    char cmdbuf_[4096];
    int cmdcnt_;
    char txbuf_[4096];
    int txcnt_;
    bool connected_;
};

DECLARE_CLASS(DpiClient)

}  // namespace debugger

#endif  // __DEBUGGER_LIBDBG64G_SERVICES_REMOTE_DPICLIENT_H__
