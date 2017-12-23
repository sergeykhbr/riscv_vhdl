/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      I2C interface description.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_II2C_H__
#define __DEBUGGER_COMMON_CORESERVICES_II2C_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

enum EStatusI2C {
    I2C_ACK   = 0,
    I2C_NACK  = 1
};

static const char *const IFACE_I2C_MASTER = "IMasterI2C";

class IMasterI2C : public IFace {
 public:
    IMasterI2C() : IFace(IFACE_I2C_MASTER) {}

    virtual void registerI2CListener(IFace *iface) = 0;
    virtual void unregisterI2CListener(IFace *iface) = 0;
};

static const char *const IFACE_I2C_SLAVE = "ISlaveI2C";

class ISlaveI2C : public IFace {
 public:
    ISlaveI2C() : IFace(IFACE_I2C_SLAVE) {}

    virtual uint8_t getPhysAddress() = 0;
    virtual uint8_t getAddressLen() = 0;
    virtual void setAddress(uint32_t addr) = 0;
    virtual EStatusI2C writeNext(uint8_t byte) = 0;
    virtual EStatusI2C readNext(uint8_t *byte) = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_II2C_H__
