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
package d2l_d_pkg;


typedef struct {
    logic busy;
    logic [2:0] ena;
    logic signA;
    logic [10:0] expA;
    logic [52:0] mantA;
    logic [63:0] result;
    logic op_signed;
    logic w32;
    logic [63:0] mantPostScale;
    logic overflow;
    logic underflow;
} Double2Long_registers;

const Double2Long_registers Double2Long_r_reset = '{
    1'b0,                               // busy
    '0,                                 // ena
    1'b0,                               // signA
    '0,                                 // expA
    '0,                                 // mantA
    '0,                                 // result
    1'b0,                               // op_signed
    1'b0,                               // w32
    '0,                                 // mantPostScale
    1'b0,                               // overflow
    1'b0                                // underflow
};

endpackage: d2l_d_pkg
