/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Optical Encoder interface.
 */

#ifndef __DEBUGGER_COMMON_CORESERVICES_IENCODER_H__
#define __DEBUGGER_COMMON_CORESERVICES_IENCODER_H__

#include <inttypes.h>
#include <iface.h>

namespace debugger {

static const char *const IFACE_ENCODER = "IEncoder";

class IEncoder : public IFace {
 public:
    IEncoder() : IFace(IFACE_ENCODER) {}

    virtual void rotateOn(int steps) = 0;
    virtual uint8_t getEncoderState() = 0;
    virtual double getAngleDegrees() = 0;
    virtual double getHoles() = 0;
    virtual double getPeriods() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IENCODER_H__
