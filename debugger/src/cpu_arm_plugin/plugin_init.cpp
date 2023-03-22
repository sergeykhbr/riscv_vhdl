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

#include "cpu_arm7_func.h"
#include "stm32l4/stm32l4_rcc.h"
#include "stm32l4/stm32l4_gpio.h"
#include "stm32l4/stm32l4_systick.h"
#include "stm32l4/stm32l4_nvic.h"
#include "stm32l4/st7789v.h"
#include "stm32l4/demo_keypad.h"

namespace debugger {

extern "C" void plugin_init(void) {
    REGISTER_CLASS_IDX(CpuCortex_Functional, 1);

    // Move into separate library stm32:
    REGISTER_CLASS_IDX(STM32L4_RCC, 3);
    REGISTER_CLASS_IDX(STM32L4_GPIO, 4);
    REGISTER_CLASS_IDX(STM32L4_SysTick, 5);
    REGISTER_CLASS_IDX(STM32L4_NVIC, 6);
    REGISTER_CLASS_IDX(ST7789V, 7);
    REGISTER_CLASS_IDX(DemoKeypad, 8);
}

}  // namespace debugger
