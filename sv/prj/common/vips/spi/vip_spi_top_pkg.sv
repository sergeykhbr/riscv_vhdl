// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
package vip_spi_top_pkg;


typedef struct {
    logic resp_valid;
    logic [31:0] resp_rdata;
    logic [31:0] scratch0;
    logic [31:0] scratch1;
    logic [31:0] scratch2;
    logic uart_loopback;
    logic [15:0] gpio_out;
    logic [15:0] gpio_dir;
} vip_spi_top_registers;

const vip_spi_top_registers vip_spi_top_r_reset = '{
    1'b0,                               // resp_valid
    '0,                                 // resp_rdata
    '0,                                 // scratch0
    '0,                                 // scratch1
    '0,                                 // scratch2
    1'b0,                               // uart_loopback
    '0,                                 // gpio_out
    '0                                  // gpio_dir
};

endpackage: vip_spi_top_pkg
