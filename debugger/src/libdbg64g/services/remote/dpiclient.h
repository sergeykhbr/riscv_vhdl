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
#include "coreservices/itap.h"

namespace debugger {

class DpiClient : public IService,
                  public IThread,
                  public IDpi,
                  public ITap {
 public:
    explicit DpiClient(const char *name);
    virtual ~DpiClient();

    /** IService interface */
    virtual void postinitService();
    virtual void predeleteService() override;

    /** IDpi */
    virtual void axi4_write(uint64_t addr, int bytes, uint64_t data);
    virtual void axi4_read(uint64_t addr, int bytes, uint64_t *data);
    virtual bool is_irq();
    virtual int get_irq();

    /** ITap interface */
    virtual int read(uint64_t addr, int bytes, uint8_t *obuf);
    virtual int write(uint64_t addr, int bytes, uint8_t *ibuf);

    /** Common methods */
    double getHartBeatTime() { return hartbeatTime_; }
    uint64_t getHartBeatClkcnt() { return hartbeatClkcnt_; }

 protected:
    /** IThread interface */
    virtual void busyLoop();

 protected:
    int createServerSocket();
    void closeServerSocket();
    void processRx();
    void processTx();
    void writeTx(const char *buf, unsigned size);
    bool syncRequest(const char *buf, unsigned size);

    void msgRead(uint64_t addr, int bytes);
    void msgWrite(uint64_t addr, int bytes, uint8_t *buf);

 private:
    static const int BURST_LEN_MAX = 4*8;    // hardcoded in libdpiwrapper

    AttributeType isEnable_;
    AttributeType cmdexec_;
    AttributeType timeout_;
    AttributeType hostIP_;
    AttributeType hostPort_;
    AttributeType syncResponse_;
    AttributeType reqHartBeat_;
    AttributeType respHartBeat_;

    ICmdExecutor *iexec_;

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
    double hartbeatTime_;
    uint64_t hartbeatClkcnt_;

    char tmpbuf_[1024];
    int tmpsz_;

};

DECLARE_CLASS(DpiClient)

}  // namespace debugger

#endif  // __DEBUGGER_LIBDBG64G_SERVICES_REMOTE_DPICLIENT_H__
