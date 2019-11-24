/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     RF Fron-end controller register mapping definition.
******************************************************************************/
#ifndef __MAP_RFCTRL_H__
#define __MAP_RFCTRL_H__

#include <inttypes.h>

#define SUBSYSTEM_RFCTRL_ENA  (1ul << 0)
#define SUBSYSTEM_ENGINE_ENA  (1ul << 1)
#define SUBSYSTEM_FSE_GPS_ENA (1ul << 2)
#define SUBSYSTEM_FSE_GLO_ENA (1ul << 3)
#define SUBSYSTEM_FSE_GAL_ENA (1ul << 4)

typedef struct rfctrl_map {
    volatile uint32_t conf1;		// 0x00
    volatile uint32_t conf2;		// 0x04
    volatile uint32_t conf3;		// 0x08
    volatile uint32_t pllconf;		// 0x0C
    volatile uint32_t div;	        // 0x10
    volatile uint32_t fdiv;	        // 0x14
    volatile uint32_t strm;	        // 0x18
    volatile uint32_t clkdiv;		// 0x1C
    volatile uint32_t test1;		// 0x20
    volatile uint32_t test2;		// 0x24
    volatile uint32_t scale;		// 0x28
    volatile uint32_t run;		    // 0x2C
    volatile uint32_t reserved1[3];	// 0x30,0x34,0x38
    volatile uint32_t rw_ant_status;// 0x3C
    volatile uint32_t subsystem_config;  // 0x40
} rfctrl_map;

#endif  // __MAP_RFCTRL_H__
