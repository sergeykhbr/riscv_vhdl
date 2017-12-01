/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Remote access to debugger via TCP connection. Client thread.
 */

#ifndef __DEBUGGER_TCPCLIENT_H__
#define __DEBUGGER_TCPCLIENT_H__

#include <iclass.h>
#include <iservice.h>
#include "tcpcmd.h"
#include "coreservices/ithread.h"
#include "coreservices/irawlistener.h"

namespace debugger {

class TcpClient : public IService,
                  public IThread,
                  public IRawListener {
 public:
    explicit TcpClient(const char *name);
    virtual ~TcpClient();

    /** IService interface */
    virtual void postinitService();
    virtual void setExtArgument(void *args) {
        hsock_ = *reinterpret_cast<socket_def *>(args);
    }

    /** IRawListener interface */
    virtual void updateData(const char *buf, int buflen);

 protected:
    /** IThread interface */
    virtual void busyLoop();

 protected:
    void processRxString();
    int sendTxBuf();
    void closeSocket();

 private:
    AttributeType isEnable_;
    AttributeType timeout_;
    socket_def hsock_;
    mutex_def mutexTx_;
    char rcvbuf[4096];
    char cmdbuf_[4096];
    int cmdcnt_;
    char txbuf_[1<<20];
    int txcnt_;

    TcpCommands tcpcmd_;
};

DECLARE_CLASS(TcpClient)

}  // namespace debugger

#endif  // __DEBUGGER_TCPCLIENT_H__
