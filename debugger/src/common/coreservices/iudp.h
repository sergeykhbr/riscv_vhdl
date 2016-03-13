/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      UDP transport interface declaration.
 */

#ifndef __DEBUGGER_IUDP_H__
#define __DEBUGGER_IUDP_H__

#include "iface.h"
#include "irawlistener.h"
#include "attribute.h"

namespace debugger {

static const char *const IFACE_UDP = "IUdp";

class IUdp : public IFace {
public:
    IUdp() : IFace(IFACE_UDP) {}

    virtual AttributeType getConnectionSettings() =0;

    virtual void setTargetSettings(const AttributeType *target) =0;

    virtual int sendData(const char *msg, int len) =0;

    virtual int readData(const char *buf, int maxlen) =0;

    virtual int registerListener(IRawListener *ilistener) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IUDP_H__
