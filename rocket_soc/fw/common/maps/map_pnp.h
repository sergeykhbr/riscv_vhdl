/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     Plug-and-Play module register mapping definition.
******************************************************************************/

#ifndef __MAP_PNP_H__
#define __MAP_PNP_H__

#include <inttypes.h>


#define PNP_CONFIG_DEFAULT_BYTES 16

typedef struct PnpConfigType {
    uint32_t xmask;
    uint32_t xaddr;
    uint16_t did;
    uint16_t vid;
    uint8_t size;
    uint8_t rsrv[3];
} PnpConfigType;

typedef struct pnp_map {
    volatile uint32_t hwid;         /// Read only HW ID
    volatile uint32_t fwid;         /// Read/Write Firmware ID
    volatile uint32_t tech;         /// Read only technology index
    volatile uint32_t rsrv1;        /// 
    volatile uint64_t idt;          /// 
    volatile uint64_t malloc_addr;  /// debuggind memalloc pointer
    volatile uint64_t malloc_size;  /// debugging memalloc size
    volatile uint64_t fwdbg1;       /// FW debug register
    volatile uint64_t rsrv[2];
    PnpConfigType slaves[64];
} pnp_map;

#endif  // __MAP_PNP_H__
