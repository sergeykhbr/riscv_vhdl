/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     AXI4 device mapping
 * @details   Don't use this address directly use Kernel interface to get
 *            detected device interface.
******************************************************************************/

#ifndef __AXI_MAPS_H__
#define __AXI_MAPS_H__

#include <inttypes.h>
#include "axi_const.h"
#include "maps/map_pnp.h"
#include "maps/map_gpio.h"
#include "maps/map_gptimers.h"
#include "maps/map_uart.h"
#include "maps/map_irqctrl.h"
#include "maps/map_rfctrl.h"
#include "maps/map_gnssengine.h"
#include "maps/map_ethmac.h"

#define ADDR_BUS0_XSLV_OTP          0x00010000
#define ADDR_BUS0_XSLV_FWIMAGE      0x00100000
#define ADDR_BUS0_XSLV_EXTFLASH     0x00200000
#define ADDR_BUS0_XSLV_SRAM         0x10000000
#define ADDR_BUS0_XSLV_GPIO         0x80000000
#define ADDR_BUS0_XSLV_UART1        0x80001000
#define ADDR_BUS0_XSLV_IRQCTRL      0x80002000
#define ADDR_BUS0_XSLV_GPTIMERS     0x80005000
#define ADDR_BUS0_XSLV_GNSS_SS      0x80008000
#define ADDR_BUS0_XSLV_ETHMAC       0x80040000
#define ADDR_BUS0_XSLV_PNP          0xfffff000
// GNSS Sub System
#define ADDR_GNSS_SS_RFCTRL         0x80008000
#define ADDR_GNSS_SS_ENGINE         0x80009000
#define ADDR_GNSS_SS_FSEGPS         0x8000a000

// Interrupt pins assignemts:
#define CFG_IRQ_UNUSED      0
#define CFG_IRQ_UART1       1
#define CFG_IRQ_ETHMAC      2
#define CFG_IRQ_GPTIMERS    3
#define CFG_IRQ_GNSSENGINE  4
#define CFG_IRQ_TOTAL       5


#endif  // __AXI_MAPS_H__
