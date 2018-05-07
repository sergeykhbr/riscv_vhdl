/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     Ethernet MAC memory map.
******************************************************************************/
#ifndef __MAP_ETHERNET_H__
#define __MAP_ETHERNET_H__

#include <inttypes.h>

typedef struct eth_map {
    volatile uint32_t control;        
    volatile uint32_t status;        
    volatile uint64_t esa;
    //volatile uint32_t esa_msb;        
    volatile uint32_t mdio;        
    volatile uint32_t tx_desc_p;        
    volatile uint32_t rx_desc_p;        
    volatile uint32_t edclip;
} eth_map;

#endif  // __MAP_ETHERNET_H__
