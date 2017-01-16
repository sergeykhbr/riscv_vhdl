/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Bus utilization computation
 *
 * @details    Read CPU dport registers: clock counter and per
 *             master counters with read/write transactions to compute
 *             utilization characteristic.
 */

#ifndef __DEBUGGER_CMD_BUSUTIL_H__
#define __DEBUGGER_CMD_BUSUTIL_H__

#include "api_core.h"
#include "coreservices/itap.h"
#include "coreservices/isocinfo.h"
#include "coreservices/icommand.h"

namespace debugger {

class CmdBusUtil : public ICommand  {
public:
    explicit CmdBusUtil(ITap *tap, ISocInfo *info);

    /** ICommand */
    virtual bool isValid(AttributeType *args);
    virtual void exec(AttributeType *args, AttributeType *res);

private:
    uint64_t clock_cnt_z_;
    DsuMapType::local_regs_type::local_region_type::mst_bus_util_type
        bus_util_z_[32];
};

}  // namespace debugger

#endif  // __DEBUGGER_CMD_BUSUTIL_H__
