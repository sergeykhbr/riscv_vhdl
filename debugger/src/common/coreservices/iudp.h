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

    /** Get opened socket connection settings. */
    virtual AttributeType getConnectionSettings() =0;

    /** Setup remote host settings */
    virtual void setTargetSettings(const AttributeType *target) =0;

    /**
     * @brief Setup socket mode.
     * @param[in] mode New value:
     *                     true: Blocking mode
     *                     false: Non-Blocking mode
     * @return true value on success.
     */
    virtual bool setBlockingMode(socket_def h, bool mode) =0;

    /** Send datagram buffer. */
    virtual int sendData(const uint8_t *msg, int len) =0;

    /** Read datagram buffer. */
    virtual int readData(const uint8_t *buf, int maxlen) =0;

    /** Register listener of the received data. */
    virtual int registerListener(IRawListener *ilistener) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IUDP_H__
