/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System Bus class declaration (AMBA or whatever).
 */

#ifndef __DEBUGGER_BUS_H__
#define __DEBUGGER_BUS_H__

#include "iclass.h"
#include "iservice.h"
#include "coreservices/iclock.h"
#include "coreservices/ibus.h"
#include <string>

namespace debugger {

class Bus : public IService,
            public IBus {
public:
    explicit Bus(const char *name);
    virtual ~Bus();

    /** IService interface */
    virtual void postinitService();

    /** IDma interface */
    virtual void map(IMemoryOperation *imemop);
    virtual int read(uint64_t addr, uint8_t *payload, int sz);
    virtual int write(uint64_t addr, uint8_t *payload, int sz);

private:
    AttributeType listMap_;
    AttributeType imap_;
    // Clock interface is used just to tag debug output with some step value,
    // in a case of several clocks the first found will be used.
    IClock *iclk0_;
};

DECLARE_CLASS(Bus)

}  // namespace debugger

#endif  // __DEBUGGER_BUS_H__
