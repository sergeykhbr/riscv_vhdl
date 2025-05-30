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
package idiv53_pkg;


typedef struct {
    logic [14:0] delay;
    logic [6:0] lshift;
    logic lshift_rdy;
    logic [52:0] divisor;
    logic [60:0] divident;
    logic [104:0] bits;
    logic overflow;
    logic zero_resid;
} idiv53_registers;

const idiv53_registers idiv53_r_reset = '{
    '0,                                 // delay
    '0,                                 // lshift
    1'b0,                               // lshift_rdy
    '0,                                 // divisor
    '0,                                 // divident
    '0,                                 // bits
    1'b0,                               // overflow
    1'b0                                // zero_resid
};

endpackage: idiv53_pkg
