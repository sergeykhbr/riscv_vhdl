/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Link interface declaration.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_ILINK_H__
#define __DEBUGGER_COMMON_CORESERVICES_ILINK_H__

#include <iface.h>
#include <attribute.h>
#include "irawlistener.h"

namespace debugger {

static const char *const IFACE_LINK = "ILink";

class ILink : public IFace {
 public:
    ILink() : IFace(IFACE_LINK) {}

    /** Get opened socket connection settings. */
    virtual void getConnectionSettings(AttributeType *settings) = 0;

    /** Setup remote host settings */
    virtual void setConnectionSettings(const AttributeType *target) = 0;

    /** Send datagram buffer. */
    virtual int sendData(const uint8_t *msg, int len) = 0;

    /** Read datagram buffer. */
    virtual int readData(const uint8_t *buf, int maxlen) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_ILINK_H__
