/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Remote access to debugger via TCP connection. Server side.
 */

#ifndef __DEBUGGER_TCPSERVER_H__
#define __DEBUGGER_TCPSERVER_H__

#include <iclass.h>
#include <iservice.h>
#include "coreservices/ithread.h"
#include "tcpclient.h"

namespace debugger {

class TcpServer : public IService,
                  public IThread {
 public:
    explicit TcpServer(const char *name);

    /** IService interface */
    virtual void postinitService();

 protected:
    /** IThread interface */
    virtual void busyLoop();

 protected:
    int createServerSocket();
    void closeServerSocket();
    void setRcvTimeout(socket_def skt, int timeout_ms);
    bool setBlockingMode(bool mode);

 private:
    AttributeType isEnable_;
    AttributeType timeout_;
    AttributeType blockmode_;
    AttributeType hostIP_;
    AttributeType hostPort_;

    struct sockaddr_in sockaddr_ipv4_;
    socket_def hsock_;
    char rcvbuf[4096];
};

DECLARE_CLASS(TcpServer)

}  // namespace debugger

#endif  // __DEBUGGER_TCPSERVER_H__
