/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Serial Interface declaration.
 */

#ifndef __DEBUGGER_ISERIAL_H__
#define __DEBUGGER_ISERIAL_H__

#include "iface.h"

namespace debugger {

static const char *IFACE_SERIAL = "ISerial";

class ISerial : public IFace {
public:
    ISerial() : IFace(IFACE_SERIAL) {}

    /**
    * @brief Write data buffer from external module.
    * @return Number of written bytes.
    */
    virtual int writeData(const char *buf, int sz) =0;

    virtual void registerRawListener(IFace *listener) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ISERIAL_H__
