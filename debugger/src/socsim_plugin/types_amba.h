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

static const uint16_t VENDOR_GNSSSENSOR        = 0x00F1;

static const uint16_t GNSSSENSOR_DUMMY         = 0x5577;
static const uint16_t GNSSSENSOR_BOOTROM       = 0x0071;
static const uint16_t GNSSSENSOR_FWIMAGE       = 0x0072;
static const uint16_t GNSSSENSOR_SRAM          = 0x0073;
static const uint16_t GNSSSENSOR_PNP           = 0x0074;
static const uint16_t GNSSSENSOR_SPI_FLASH     = 0x0075;
static const uint16_t GNSSSENSOR_GPIO          = 0x0076;
static const uint16_t GNSSSENSOR_RF_CONTROL    = 0x0077;
static const uint16_t GNSSSENSOR_ENGINE        = 0x0078;
static const uint16_t GNSSSENSOR_ENGINE_STUB   = 0x0068;
static const uint16_t GNSSSENSOR_FSE_V2        = 0x0079;
static const uint16_t GNSSSENSOR_UART          = 0x007a;
static const uint16_t GNSSSENSOR_ACCELEROMETER = 0x007b;
static const uint16_t GNSSSENSOR_GYROSCOPE     = 0x007c;
static const uint16_t GNSSSENSOR_IRQCTRL       = 0x007d;

static const uint8_t PNP_CONFIG_DEFAULT_BYTES  = 16;

static const uint32_t TECH_INFERRED            = 0;
static const uint32_t TECH_VIRTEX6             = 36;
static const uint32_t TECH_KINTEX7             = 49;


}  // namespace debugger

#endif  // __DEBUGGER_SOCSIM_TYPES_AMBA_H__
