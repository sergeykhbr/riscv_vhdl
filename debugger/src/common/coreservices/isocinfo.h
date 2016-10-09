/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      SOC Information interface.
 */

#ifndef __DEBUGGER_ISOCINFO_H__
#define __DEBUGGER_ISOCINFO_H__

#include "iface.h"
#include "attribute.h"

namespace debugger {

static const char *IFACE_SOC_INFO = "ISocInfo";

struct GpioType {
    union {
        struct MapType {
            uint32_t led;
            uint32_t dip;
        } map;
        uint64_t val[1];
        uint8_t buf[8];
    } u;
};

union DsuRunControlRegType {
    struct bits_type {
        uint64_t halt     : 1;
        uint64_t stepping : 1;
        uint64_t rsv1     : 2;
        uint64_t core_id  : 16;
        uint64_t rsv2     : 44;
    } bits;
    uint64_t val;
    uint8_t  buf[8];
};

class ISocInfo : public IFace {
public:
    ISocInfo() : IFace(IFACE_SOC_INFO) {}

    virtual unsigned getRegsTotal() =0;
    virtual void getRegsList(AttributeType *lst) =0;
    virtual unsigned getCsrTotal() =0;
    virtual void getCsrList(AttributeType *lst) =0;
    virtual uint64_t csr2addr(const char *name) =0;
    virtual uint64_t reg2addr(const char *name) =0;

    virtual uint64_t addressPlugAndPlay() =0;
    virtual uint64_t addressGpio() =0;
    virtual uint64_t addressBreakCreate() =0;
    virtual uint64_t addressBreakRemove() =0;
    virtual uint64_t addressRunControl() =0;
    virtual uint64_t addressStepCounter() =0;
    virtual uint64_t valueHalt() =0;
    virtual uint64_t valueRun() =0;
    virtual uint64_t valueRunStepping() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_ISOCINFO_H__
