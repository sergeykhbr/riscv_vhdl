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
    volatile uint32_t input_val;  // [0x00] Pin value
    volatile uint32_t input_en;   // [0x04] Pin input enable
    volatile uint32_t output_en;  // [0x08] Pin output enable
    volatile uint32_t output_val; // [0x0C] Output value
    volatile uint32_t pue;        // [0x10] internal pull-up enable
    volatile uint32_t ds;         // [0x14] Pin drive strength
    volatile uint32_t rise_ie;    // [0x18] Rise interrupt enable
    volatile uint32_t rise_ip;    // [0x1C] Rise interrupt pending
    volatile uint32_t fall_ie;    // [0x20] Fall interrupt enable
    volatile uint32_t fall_ip;    // [0x24] Fall interrupt pending
    volatile uint32_t high_ie;    // [0x28] High interrupt enable
    volatile uint32_t high_ip;    // [0x2C] High interrupt pending
    volatile uint32_t low_ie;     // [0x30] Low interrupt enable
    volatile uint32_t low_ip;     // [0x34] Low interrupt pending
    volatile uint32_t iof_en;     // [0x38] IO function enable
    volatile uint32_t iof_sel;    // [0x3C] IO function select
    volatile uint32_t out_xor;    // [0x40] Output XOR (invert)
} gpio_map;

#endif  // __MAP_GPIO_H__
