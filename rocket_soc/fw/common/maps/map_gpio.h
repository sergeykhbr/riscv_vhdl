/******************************************************************************
 * @file
 * @copyright Copyright 2015 GNSS Sensor Ltd. All right reserved.
 * @author    Sergey Khabarov - sergeykhbr@gmail.com
 * @brief     GPIO register mapping definition.
******************************************************************************/

#ifndef __MAP_GPIO_H__
#define __MAP_GPIO_H__

#include <inttypes.h>

typedef struct gpio_map {
    volatile uint32_t led;
    volatile uint32_t dip;
    volatile uint32_t reg2;
    volatile uint32_t reg3;
    volatile uint32_t led_period;
    volatile uint32_t reg5;
    volatile uint32_t reg6;
} gpio_map;

#endif  // __MAP_GPIO_H__
