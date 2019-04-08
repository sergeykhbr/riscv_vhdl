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

#ifndef __DEBUGGER_SRC_LIBDBG64G_SERVICES_DEBUG_UDP_DBGLINK_H__
#define __DEBUGGER_SRC_LIBDBG64G_SERVICES_DEBUG_UDP_DBGLINK_H__

#include "iclass.h"
#include "iservice.h"
#include <ihap.h>
#include "coreservices/ilink.h"
#include "coreservices/itap.h"

namespace debugger {

class UdpService : public IService,
                   public ILink,
                   public IHap {
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

    /** IHap */
    virtual void hapTriggered(IFace *isrc, EHapType type, const char *descr);

 protected:
    int createDatagramSocket();
    void closeDatagramSocket();
    bool setBlockingMode(bool mode);

 private:
    AttributeType timeout_;
    AttributeType blockmode_;
    AttributeType hostIP_;
    AttributeType boardIP_;
    AttributeType simTarget_;
    
    struct sockaddr_in sockaddr_ipv4_;
    char               sockaddr_ipv4_str_[16];
    unsigned short     sockaddr_ipv4_port_;
    struct sockaddr_in remote_sockaddr_ipv4_;
    socket_def hsock_;
    char rcvbuf[4096];
};

DECLARE_CLASS(UdpService)

}  // namespace debugger

#endif  // __DEBUGGER_SRC_LIBDBG64G_SERVICES_DEBUG_UDP_DBGLINK_H__
