/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Simulator of the FPGA board with Ethernet UDP/EDCL interface.
 */

#ifndef __DEBUGGER_IBOARD_SIM_H__
#define __DEBUGGER_IBOARD_SIM_H__

#include "iface.h"
#include "attribute.h"

namespace debugger {

static const char *const IFACE_BOARDSIM = "IBoardSim";

class IBoardSim : public IFace {
public:
    IBoardSim() : IFace(IFACE_BOARDSIM) {}

    virtual const char *getBrief() { 
        return "FPGA development board simulator interface";
    }

    virtual const char *getDetail() {
        return "This interface declares functionality of the emulator of the "
               "real hardware. Such emulator allows to develop network "
               "interfaces (UDP/EDCL) without connection to the FPGA that "
               "significantly simplify debugging.";
    }

    virtual void getInfo(AttributeType *attr) =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_IBOARD_SIM_H__
