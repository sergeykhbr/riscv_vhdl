/*
 *  Copyright 2023 Sergey Khabarov, sergeykhbr@gmail.com
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

#include <inttypes.h>
#include <axi_maps.h>

void __attribute__((weak)) hwinit() {
    uart_map *uart = (uart_map *)ADDR_BUS0_XSLV_UART0;

    // scaler is enabled in SRAM self test, duplicate it here
    uart->scaler = SYS_HZ / 115200 / 2;
    uart->txctrl = 0x1;  // txen=1
    uart->rxctrl = 0x1;  // rxen=1
    uart_irq_type ie;
    ie.v = 0;
#ifdef UART_BUF_ENABLE    
    ie.b.txwm = 1;
    ie.b.rxwm = 0;
#endif
    uart->ie = ie.v;
}
