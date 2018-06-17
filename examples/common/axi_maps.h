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

#define ADDR_NASTI_SLAVE_FWIMAGE    0x00100000
#define ADDR_NASTI_SLAVE_SRAM       0x10000000
#define ADDR_NASTI_SLAVE_GPIO       0x80000000
#define ADDR_NASTI_SLAVE_UART1      0x80001000
#define ADDR_NASTI_SLAVE_IRQCTRL    0x80002000
#define ADDR_NASTI_SLAVE_GNSSENGINE 0x80003000
#define ADDR_NASTI_SLAVE_RFCTRL     0x80004000
#define ADDR_NASTI_SLAVE_GPTIMERS   0x80005000
#define ADDR_NASTI_SLAVE_FSEGPS     0x8000a000
#define ADDR_NASTI_SLAVE_ETHMAC     0x80040000
#define ADDR_NASTI_SLAVE_PNP        0xfffff000


// Interrupt pins assignemts:
#define CFG_IRQ_UNUSED      0
#define CFG_IRQ_UART1       1
#define CFG_IRQ_ETHMAC      2
#define CFG_IRQ_GPTIMERS    3
#define CFG_IRQ_MISS_ACCESS 4
#define CFG_IRQ_GNSSENGINE  5
#define CFG_IRQ_TOTAL       6


#endif  // __AXI_MAPS_H__
