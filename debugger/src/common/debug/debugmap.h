/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Debug Support Unit (DSU) memory map.
 */

#ifndef __DEBUGGER_COMMON_DEBUG_DEBUGMAP_H__
#define __DEBUGGER_COMMON_DEBUG_DEBUGMAP_H__

#include <inttypes.h>

namespace debugger {

union GenericCpuControlType {
    uint64_t val;
    struct {
        uint64_t halt     : 1;
        uint64_t stepping : 1;
        uint64_t sw_breakpoint : 1;
        uint64_t hw_breakpoint : 1;
        uint64_t core_id  : 16;
        uint64_t rsv2     : 12;
        uint64_t istate   : 2;  // [33:32] icache state
        uint64_t rsv3     : 2;  // [35:34] 
        uint64_t dstate   : 2;  // [37:36] dcache state
        uint64_t rsv4     : 2;  // [39:38]
        uint64_t cstate   : 2;  // [41:40] cachetop state
        uint64_t rsv5     : 22;
    } bits;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_DEBUG_DEBUGMAP_H__
