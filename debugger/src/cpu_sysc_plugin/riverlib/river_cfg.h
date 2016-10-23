/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      "River" CPU configuration parameters
 */

#ifndef __DEBUGGER_RIVER_CFG_H__
#define __DEBUGGER_RIVER_CFG_H__

#include <systemc.h>

namespace debugger {

#define GENERATE_VCD 0

static const int RISCV_ARCH     = 64;

static const int AXI_ADDR_WIDTH = 32;
static const int AXI_DATA_WIDTH = 64;
static const int AXI_DATA_BYTES = AXI_DATA_WIDTH / 8;

static const uint64_t RESET_VECTOR = 0x00001000;


}  // namespace debugger

#endif  // __DEBUGGER_RIVER_CFG_H__
