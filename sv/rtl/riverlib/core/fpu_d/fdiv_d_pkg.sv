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
package fdiv_d_pkg;


typedef struct {
    logic busy;
    logic [4:0] ena;
    logic [63:0] a;
    logic [63:0] b;
    logic [63:0] result;
    logic zeroA;
    logic zeroB;
    logic [52:0] divisor;
    logic [5:0] preShift;
    logic [12:0] expAB;
    logic [11:0] expAlign;
    logic [104:0] mantAlign;
    logic [11:0] postShift;
    logic [104:0] mantPostScale;
    logic nanRes;
    logic overflow;
    logic underflow;
    logic illegal_op;
} DoubleDiv_registers;

const DoubleDiv_registers DoubleDiv_r_reset = '{
    1'b0,                               // busy
    '0,                                 // ena
    '0,                                 // a
    '0,                                 // b
    '0,                                 // result
    1'b0,                               // zeroA
    1'b0,                               // zeroB
    '0,                                 // divisor
    '0,                                 // preShift
    '0,                                 // expAB
    '0,                                 // expAlign
    '0,                                 // mantAlign
    '0,                                 // postShift
    '0,                                 // mantPostScale
    1'b0,                               // nanRes
    1'b0,                               // overflow
    1'b0,                               // underflow
    1'b0                                // illegal_op
};

endpackage: fdiv_d_pkg
