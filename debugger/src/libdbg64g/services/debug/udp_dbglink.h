/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      UDP transport level implementation.
 */

#ifndef __DEBUGGER_UDP_DBGLINK_SERVICE_H__
#define __DEBUGGER_UDP_DBGLINK_SERVICE_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/ilink.h"
#include "coreservices/itap.h"

namespace debugger {

class UdpService : public IService,
                   public ILink {
public:
    UdpService(const char *name);
    ~UdpService();

    /** IService interface */
    virtual void postinitService();

    /** ILink interface */
    virtual void getConnectionSettings(AttributeType *settings);
    virtual void setConnectionSettings(const AttributeType *target);
    virtual int sendData(const uint8_t *msg, int len);
    virtual int readData(const uint8_t *buf, int maxlen);

protected:
    int createDatagramSocket();
    void closeDatagramSocket();
    bool setBlockingMode(bool mode);

private:
    AttributeType timeout_;
    AttributeType blockmode_;
    AttributeType hostIP_;
    AttributeType boardIP_;
    
    struct sockaddr_in sockaddr_ipv4_;
    char               sockaddr_ipv4_str_[16];    // 3 dots  + 4 digits each 3 symbols + '\0' = 4*3 + 3 + 1;
    unsigned short     sockaddr_ipv4_port_;
    struct sockaddr_in remote_sockaddr_ipv4_;
    socket_def hsock_;
    char rcvbuf[4096];
};

DECLARE_CLASS(UdpService)

}  // namespace debugger

#endif  // __DEBUGGER_UDP_DBGLINK_SERVICE_H__
