/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __MAP_GPIO_H__
#define __MAP_GPIO_H__

#include <inttypes.h>

typedef struct gpio_map {
    volatile uint32_t direction;  // Need to initialize 0x00F
    volatile uint32_t iuser;      // [3:0] DIPs
    volatile uint32_t ouser;      // [11:4] LEDs
    volatile uint32_t reg3;
} gpio_map;

#endif  // __MAP_GPIO_H__
