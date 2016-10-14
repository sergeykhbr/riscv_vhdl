/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      System Bus configuration types declaration.
 */

#ifndef __DEBUGGER_SOCSIM_TYPES_AMBA_H__
#define __DEBUGGER_SOCSIM_TYPES_AMBA_H__

#include <inttypes.h>

namespace debugger {

static const uint64_t CFG_NASTI_DATA_BITS     = 128;
static const uint64_t CFG_NASTI_DATA_BYTES    = CFG_NASTI_DATA_BITS / 8;
static const uint64_t CFG_NASTI_DATA_WORDS32  = CFG_NASTI_DATA_BYTES / 4;
static const uint64_t CFG_NASTI_ADDR_BITS     = 32;
static const uint64_t CFG_NASTI_ADDR_OFFSET   = 4;
static const uint64_t CFG_NASTI_CFG_ADDR_BITS = CFG_NASTI_ADDR_BITS - 12;

static const int CFG_NASTI_BOOTROM = 0;
static const int CFG_NASTI_FWROM = CFG_NASTI_BOOTROM + 1;
static const int CFG_NASTI_SRAM = CFG_NASTI_FWROM + 1;
static const int CFG_NASTI_UART = CFG_NASTI_SRAM + 1;
static const int CFG_NASTI_GPIO = CFG_NASTI_UART + 1;
static const int CFG_NASTI_IRQCTRL = CFG_NASTI_GPIO + 1;
static const int CFG_NASTI_GNSSENGINE = CFG_NASTI_IRQCTRL + 1;
static const int CFG_NASTI_RFCTRL = CFG_NASTI_GNSSENGINE + 1;
static const int CFG_NASTI_FSE_GPS = CFG_NASTI_RFCTRL + 1;
static const int CFG_NASTI_ETHMAC = CFG_NASTI_FSE_GPS + 1;
static const int CFG_NASTI_DSU = CFG_NASTI_ETHMAC + 1;
static const int CFG_NASTI_PNP = CFG_NASTI_DSU + 1;
static const int CFG_NASTI_SLAVES_TOTAL = CFG_NASTI_PNP + 1;


}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_TYPES_AMBA_H__
