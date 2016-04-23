/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      HostIO bus interface.
 */

#ifndef __DEBUGGER_PLUGIN_IHOSTIO_H__
#define __DEBUGGER_PLUGIN_IHOSTIO_H__

#include "iface.h"
#include <inttypes.h>
#include "coreservices/icpuriscv.h"

namespace debugger {

static const char *const IFACE_HOSTIO = "IHostIO";

class IHostIO : public IFace {
public:
    IHostIO() : IFace(IFACE_HOSTIO) {}

    /**
     * @return Response error code
     */
    virtual uint64_t write(uint16_t adr, uint64_t val) =0;

    virtual uint64_t read(uint16_t adr, uint64_t *val) =0;

    /**
     * CPU Debug interface (only for simulator)
     */ 
     virtual ICpuRiscV *getCpuInterface() =0;
};

}  // namespace debugger

#endif  // __DEBUGGER_PLUGIN_IHOSTIO_H__
