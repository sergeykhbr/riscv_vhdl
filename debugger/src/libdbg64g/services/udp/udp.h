/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      UDP transport level implementation.
 */

#ifndef __DEBUGGER_UDP_SERVICE_H__
#define __DEBUGGER_UDP_SERVICE_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/iudp.h"
#include <vector>

namespace debugger {

class UdpService : public IService, 
                   public IUdp {
public:
    UdpService(const char *name);
    ~UdpService();

    /** IService interface */
    virtual void postinitService();

    /** IUdp interface */
    virtual AttributeType getConnectionSettings() {
        AttributeType ret;
        ret.make_dict();
        ret["IP"] = AttributeType(inet_ntoa(sockaddr_ipv4_.sin_addr));
        ret["Port"] = AttributeType(Attr_UInteger, 
                    static_cast<uint64_t>(sockaddr_ipv4_.sin_port));
        return ret;
    }

    virtual void setTargetSettings(const AttributeType *target) {
        if (!target->is_dict()) {
            return;
        }
        remote_sockaddr_ipv4_.sin_addr.s_addr = 
            inet_addr((*target)["IP"].to_string());
        remote_sockaddr_ipv4_.sin_port = 
            static_cast<uint16_t>((*target)["Port"].to_uint64());
    }
    /**
     * @brief Setup socket mode.
     * @param[in] mode New value:
     *                     true: Blocking mode
     *                     false: Non-Blocking mode
     * @return true value on success.
     */
    virtual bool setBlockingMode(socket_def h, bool mode);

    virtual int sendData(const uint8_t *msg, int len);

    virtual int readData(const uint8_t *buf, int maxlen);

    virtual int registerListener(IRawListener *ilistener);

protected:
    int createDatagramSocket();
    void closeDatagramSocket();

private:
    std::vector<IRawListener *> vecListeners_;
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

#endif  // __DEBUGGER_UDP_SERVICE_H__
